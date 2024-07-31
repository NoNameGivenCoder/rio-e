#include <helpers/editor/EditorMgr.h>
#include <helpers/properties/Property.h>
#include <helpers/properties/audio/AudioProperty.h>
#include <string>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <gfx/rio_PrimitiveRenderer.h>
#include <helpers/common/NodeMgr.h>
#include <gfx/rio_Window.h>
#include <iostream>
#include <gpu/rio_RenderBuffer.h>
#include <misc/rio_MemUtil.h>

EditorMgr *EditorMgr::mInstance = nullptr;

bool EditorMgr::createSingleton()
{
    if (mInstance)
        return false;

    mInstance = new EditorMgr();
    mInstance->mInitialized = true;

    if (!mInstance->mInitialized)
    {
        delete mInstance;
        mInstance = nullptr;
        return false;
    }

    return true;
}

bool EditorMgr::destorySingleton()
{
    if (!mInstance)
        return false;

    delete mInstance->mpColorTexture;
    delete mInstance->mpItemIDTexture;
    delete mInstance->mpDepthTexture;
    delete mInstance->mpItemIDReadBuffer;
    delete mInstance->mpItemIDClearBuffer;

    delete mInstance;
    mInstance = nullptr;

    return true;
}

void EditorMgr::SetupFrameBuffer()
{
    rio::Window *window = rio::Window::instance();

    s32 width, height;

    width = window->getWidth();
    height = window->getHeight();

    mpColorTexture = new rio::Texture2D(rio::TEXTURE_FORMAT_R8_G8_B8_A8_UNORM, width, height, 1);
    mpItemIDTexture = new rio::Texture2D(rio::TEXTURE_FORMAT_R32_UINT, width, height, 1);
    mpDepthTexture = new rio::Texture2D(rio::DEPTH_TEXTURE_FORMAT_R32_FLOAT, width, height, 1);

    u32 size = width * height * sizeof(u32);
    mpItemIDReadBuffer = new u8[size];
    mpItemIDClearBuffer = new u8[size];

    rio::MemUtil::set(mpItemIDReadBuffer, 0xFF, size);
    RIO_ASSERT(size == mpItemIDTexture->getNativeTexture().surface.imageSize);
    rio::MemUtil::set(mpItemIDClearBuffer, 0xFF, size);

    mRenderBuffer.setSize(width, height);
    mColorTarget.linkTexture2D(*mpColorTexture);
    mDepthTarget.linkTexture2D(*mpDepthTexture);
    mItemIDTarget.linkTexture2D(*mpItemIDTexture);

    mRenderBuffer.clear(rio::RenderBuffer::CLEAR_FLAG_DEPTH);

    RIO_LOG("[EDITORMGR] Created render buffer! \n");
}

void EditorMgr::BindRenderBuffer()
{
    rio::Window *window = rio::Window::instance();

    int width, height;

    width = window->getWidth();
    height = window->getHeight();

    mRenderBuffer.setSize(width, height);

    mRenderBuffer.setRenderTargetColorNull(2);
    mRenderBuffer.bind();
}

void EditorMgr::UnbindRenderBuffer()
{
    mRenderBuffer.getRenderTargetColor()->invalidateGPUCache();
    mpColorTexture->setCompMap(0x00010205);

    rio::Window::instance()->makeContextCurrent();

    u32 width = rio::Window::instance()->getWidth();
    u32 height = rio::Window::instance()->getHeight();

    rio::Graphics::setViewport(0, 0, width, height);
    rio::Graphics::setScissor(0, 0, width, height);
}

void EditorMgr::CreateEditorUI()
{
    if (!&io)
        io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Create New Scene"))
                    NodeMgr::instance()->ClearAllNodes();

                if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
                    NodeMgr::instance()->SaveToFile();

                ImGui::MenuItem("Open Scene", "Ctrl+O");
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Create Node"))
                {
                    std::string nodeKey = "Node (" + std::to_string(NodeMgr::instance()->GetNodeCount() + 1) + ")";
                    rio::Vector3f defaultRotAndPos = {0, 0, 0};
                    rio::Vector3f defaultScale = {1, 1, 1};
                    auto createdNode = std::make_shared<Node>(nodeKey, defaultRotAndPos, defaultRotAndPos, defaultScale);
                    NodeMgr::instance()->AddNode(createdNode);
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        ImGuiID dockspace_id = ImGui::GetID("Dockspace");
        ImGuiViewport *viewport = ImGui::GetMainViewport();

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, 0});
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {0, 0});
        ImGui::Begin("Dockspace", nullptr, windowFlags);
        ImGui::PopStyleVar(5);

        ImGui::DockSpace(dockspace_id, {0, 0});

        if (ImGui::Begin("Layout", nullptr, ImGuiWindowFlags_NoCollapse))
        {
            if (ImGui::BeginChild("nodes", ImGui::GetContentRegionAvail(), ImGuiChildFlags_AutoResizeY))
            {
                for (const auto &node : NodeMgr::instance()->mNodes)
                {
                    if (!node || !node->nodeKey.c_str())
                        continue;

                    bool isNodeSelected = (EditorMgr::instance()->selectedNode == node);

                    if (isNodeSelected)
                        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);

                    if (ImGui::Button(node->nodeKey.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 22)))
                        EditorMgr::instance()->selectedNode = node;

                    if (isNodeSelected)
                        ImGui::PopStyleColor();
                }

                ImGui::EndChild();
            }

            ImGui::End();
        }

        if (ImGui::Begin("Task View"))
        {
            // Get the size of the window
            ImVec2 windowSize = ImGui::GetContentRegionAvail();

            // Display the texture
            ImGui::Image((void *)mpColorTexture->getNativeTextureHandle(), {mRenderBuffer.getSize().x, mRenderBuffer.getSize().y}, ImVec2(0, 1), ImVec2(1, 0));

            ImGui::End();
        }

        ImGui::End();
    }

    ImGui::ShowDemoWindow();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorMgr::CreateNodePropertiesMenu()
{
    // ImGui::InputText(EditorMgr::instance()->selectedNode->nodeKey, EditorMgr::instance()->selectedNode->nodeKey, sizeof(EditorMgr::instance()->selectedNode->nodeKey));

    std::shared_ptr<Node> selectedNode = EditorMgr::instance()->selectedNode;

    selectedNode->CreateNodeProperties();

    if (selectedNode->properties.size() > 0)
    {
        if (ImGui::CollapsingHeader("Properties"))
        {
            for (const auto &property : selectedNode->properties)
                property->CreatePropertiesMenu();
        }
    }
}