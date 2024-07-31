#include <helpers/properties/MiiHeadProperty.h>
#include <helpers/common/FFLMgr.h>
#include <helpers/common/NodeMgr.h>
#include <gpu/rio_RenderState.h>
#include <gpu/rio_Shader.h>
#include <gfx/rio_Window.h>
#include <gfx/rio_Graphics.h>
#include <helpers/editor/EditorMgr.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <cstdint>

MiiHeadProperty::~MiiHeadProperty()
{
    FFLDeleteCharModel(&mCharModel);
}

YAML::Node MiiHeadProperty::Save()
{
    YAML::Node node;

    node["MiiHead"]["miiDataFile"] = mMiiDataFile;
    node["MiiHead"]["propertyId"] = GetPropertyID();

    return node;
}

void MiiHeadProperty::LoadStoreData()
{
    rio::RawErrorCode errorCode;
    mStoreData = FFLMgr::instance()->GetStoreDataFromFile(mMiiDataFile, &errorCode);

    if (sizeof(mStoreData) != sizeof(FFLStoreData))
    {
        RIO_LOG("[MIIHEAD] FFLStoreData size does not match with current store data size!!");
        return;
    }

    GetAdditionalData();
}

void MiiHeadProperty::Load(YAML::Node node)
{
    mMiiDataFile = node["miiDataFile"].as<std::string>();
    SetPropertyID(node["propertyId"].as<int>());
}

void MiiHeadProperty::Start()
{
    LoadStoreData();

    // TODO: make most of this customizable.

    mCharModelSource.dataSource = FFL_DATA_SOURCE_STORE_DATA;
    mCharModelSource.pBuffer = &mStoreData;
    mCharModelSource.index = 0;
    mCharModelDesc.resolution = FFLMgr::instance()->GetGlobalResolution();
    mCharModelDesc.expressionFlag = FFL_EXPRESSION_FLAG_NORMAL;
    mCharModelDesc.resourceType = FFL_RESOURCE_TYPE_HIGH;
    mCharModelDesc.modelFlag = 1 << 0 | 1 << 1 | 1 << 2;

    if (!FFLInitCharModelCPUStep(&mCharModel, &mCharModelSource, &mCharModelDesc) == FFL_RESULT_OK)
    {
        RIO_LOG("[MIIHEAD] InitCharModelCPUStep failed!!\n");
        return;
    }

    mShader.initialize();
    mShader.bind(false);

    mpShader = &mShader;

    FFLInitCharModelGPUStep(&mCharModel);
    rio::Window::instance()->makeContextCurrent();

    auto mainCamera = NodeMgr::instance()->GetNodeByKey("mapCamera");

    mainCameraProperty = mainCamera->GetProperty<CameraProperty>().at(0);

    mInitialized = true;
}

void MiiHeadProperty::Update()
{
    EditorMgr::instance()->BindRenderBuffer();
    rio::BaseMtx34f viewMtx;
    rio::BaseMtx44f projMtx;
    rio::Mtx34f nodeMtx;

    mainCameraProperty->GetCamera().getMatrix(&viewMtx);
    projMtx = mainCameraProperty->GetProjectionMatrix();
    nodeMtx.makeSRT(GetParentNode().lock()->GetScale() / 32.f, GetParentNode().lock()->GetRotation(), GetParentNode().lock()->GetPosition());

    mpShader->bind(true);
    mpShader->setViewUniform(nodeMtx, viewMtx, projMtx);

    DrawOpa();
    DrawXlu();

    EditorMgr::instance()->UnbindRenderBuffer();
}

void MiiHeadProperty::DrawOpa()
{
    rio::RenderState render_state;
    render_state.setDepthEnable(true, true);
    render_state.setDepthFunc(rio::Graphics::COMPARE_FUNC_LEQUAL);
    render_state.setBlendEnable(false);
    render_state.setColorMask(true, true, true, true);
    render_state.apply();

    FFLDrawOpa(&mCharModel);
}

void MiiHeadProperty::DrawXlu()
{
    {
        rio::RenderState render_state;
        render_state.setDepthEnable(true, false);
        render_state.setDepthFunc(rio::Graphics::COMPARE_FUNC_LEQUAL);
        render_state.setBlendEnable(true);
        render_state.setBlendFactorSrcRGB(rio::Graphics::BLEND_MODE_SRC_ALPHA);
        render_state.setBlendFactorDstRGB(rio::Graphics::BLEND_MODE_ONE_MINUS_SRC_ALPHA);
        render_state.setBlendEquation(rio::Graphics::BLEND_FUNC_ADD);
        render_state.setColorMask(true, true, true, false);
        render_state.apply();

        mpShader->applyAlphaTestEnable();

        FFLDrawXlu(&mCharModel);
    }

    {
        rio::RenderState render_state;
        render_state.setDepthEnable(true, true);
        render_state.setDepthFunc(rio::Graphics::COMPARE_FUNC_LEQUAL);
        render_state.setBlendEnable(true);
        render_state.setBlendFactorSrcRGB(rio::Graphics::BLEND_MODE_ONE_MINUS_DST_ALPHA);
        render_state.setBlendFactorDstRGB(rio::Graphics::BLEND_MODE_DST_ALPHA);
        render_state.setBlendFactorSrcAlpha(rio::Graphics::BLEND_MODE_ONE);
        render_state.setBlendFactorDstAlpha(rio::Graphics::BLEND_MODE_ONE);
        render_state.setBlendEquation(rio::Graphics::BLEND_FUNC_ADD);
        render_state.setColorMask(true, true, true, true);
        render_state.apply();

        mpShader->applyAlphaTestEnable();

        FFLDrawXlu(&mCharModel);
    }
}

void MiiHeadProperty::CreatePropertiesMenu()
{
    std::string label = "MiiHead (" + std::to_string(GetPropertyID()) + ")";

    if (ImGui::CollapsingHeader(label.c_str()))
    {
        int currentIndex = -1;
        for (int i = 0; i < FFL_EXPRESSION_MAX; ++i)
        {
            if (expressionEnumFlags[i].value == mCharModelDesc.expressionFlag)
            {
                currentIndex = i;
                break;
            }
        }

        const char *currentName = (currentIndex != -1) ? expressionEnumFlags[currentIndex].name : "Unknown Expression";

        if (ImGui::BeginCombo("Expression", currentName))
        {
            for (int i = 0; i < FFL_EXPRESSION_MAX; i++)
            {
                bool isSelected = (mCharModelDesc.expressionFlag == 1 << i);

                if (ImGui::Selectable(expressionEnumFlags[i].name, isSelected))
                {
                    SetExpression((FFLExpressionFlag)(1 << i));
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                };
            }
        }
    };
}

void MiiHeadProperty::GetAdditionalData()
{
    FFLGetAdditionalInfo(&mAdditionalInfo, FFL_DATA_SOURCE_STORE_DATA, &mStoreData, 0, 0);

    for (auto character : mAdditionalInfo.name)
        mMiiName.push_back(static_cast<char>(character));
}