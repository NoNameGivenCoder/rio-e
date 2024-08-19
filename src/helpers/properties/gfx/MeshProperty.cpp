#include <helpers/properties/gfx/MeshProperty.h>
#include <helpers/common/NodeMgr.h>

__attribute__((aligned(rio::Drawer::cUniformBlockAlignment))) MeshProperty::ViewBlock MeshProperty::sViewBlock;
__attribute__((aligned(rio::Drawer::cUniformBlockAlignment))) MeshProperty::LightBlock MeshProperty::sLightBlock;

void MeshProperty::Load(YAML::Node node)
{
    mMeshFileName = node["meshFileName"].as<std::string>();
    mMeshKey = node["meshKey"].as<std::string>();
}

void MeshProperty::LoadMesh()
{
    rio::mdl::res::Model *resModel = rio::mdl::res::ModelCacher::instance()->loadModel(mMeshFileName.c_str(), mMeshKey.c_str());

    if (!resModel)
    {
        RIO_LOG("[MESH] Failed to load %s!\n", mMeshFileName);
        return;
    }

    mMdlModel = std::make_unique<rio::mdl::Model>(resModel);
}

MeshProperty::~MeshProperty()
{
    rio::MemUtil::free(mModelUniformBlock);
    rio::MemUtil::free(mModelBlock);
    rio::MemUtil::free(mUniformBlocks);

    delete mpLightUniformBlock;
    delete mpViewUniformBlock;
}

void MeshProperty::Start()
{
    LoadMesh();

    if (!mMdlModel)
        return;

    rio::Mtx34f nodeMtx;
    nodeMtx.makeSRT(GetParentNode().lock()->GetScale(), GetParentNode().lock()->GetRotation(), GetParentNode().lock()->GetPosition());

    mMdlModel->setModelWorldMtx(nodeMtx);

    mpViewUniformBlock = new rio::UniformBlock();
    mpViewUniformBlock->setData(&sViewBlock, sizeof(ViewBlock));

    mCameraProperty = NodeMgr::instance()->GetGlobalCamera();

    rio::Color4f lightColor = {1, 1, 1, 1};
    sLightBlock.light_color = {lightColor.r, lightColor.g, lightColor.b};
    sLightBlock.light_pos = {0, 0, 0};

    mpLightUniformBlock = new rio::UniformBlock();
    mpLightUniformBlock->setDataInvalidate(&sLightBlock, sizeof(LightBlock));

    u32 num_meshes = mMdlModel->numMeshes();

    mModelUniformBlock = (rio::UniformBlock *)rio::MemUtil::alloc(num_meshes * sizeof(rio::UniformBlock), 4);
    mModelBlock = (ModelBlock *)rio::MemUtil::alloc(num_meshes * sizeof(ModelBlock), rio::Drawer::cUniformBlockAlignment);
    mUniformBlocks = (UniformBlocks *)rio::MemUtil::alloc(num_meshes * sizeof(UniformBlocks), 4);

    for (u32 i = 0; i < num_meshes; i++)
    {
        const rio::mdl::Mesh *p_mesh = &(mMdlModel->meshes()[i]);
        const rio::mdl::Material *p_material = p_mesh->material();

        ShaderLocation view_block_idx;
        ShaderLocation light_block_idx;
        ShaderLocation model_block_idx;

        if (p_material)
        {
            rio::Shader *p_shader = p_material->shader();

            if (!p_shader)
                continue;

            view_block_idx.vs = p_shader->getVertexUniformBlockIndex("cViewBlock");
            view_block_idx.fs = p_shader->getFragmentUniformBlockIndex("cViewBlock");
            view_block_idx.findStage();

            light_block_idx.vs = p_shader->getVertexUniformBlockIndex("cLightBlock");
            light_block_idx.fs = p_shader->getFragmentUniformBlockIndex("cLightBlock");
            light_block_idx.findStage();

            model_block_idx.vs = p_shader->getVertexUniformBlockIndex("cModelBlock");
            model_block_idx.fs = p_shader->getFragmentUniformBlockIndex("cModelBlock");
            model_block_idx.findStage();
        }

        new (&mModelUniformBlock[i]) rio::UniformBlock(model_block_idx.stage, model_block_idx.vs, model_block_idx.fs);
        mModelUniformBlock[i].setData(&mModelBlock[i], sizeof(ModelBlock));

        new (&mUniformBlocks[i]) UniformBlocks(view_block_idx, light_block_idx);
    }
}

void MeshProperty::Update()
{
    if (!mCameraProperty)
        return;

    sLightBlock.light_color = {1, 1, 1};
    sLightBlock.light_pos = {0, 0, 0};

    rio::Matrix34f view_mtx;
    rio::Matrix44f view_proj_mtx;

    mCameraProperty->GetCamera().getMatrix(&view_mtx);

    // Calculate view-projection matrix (Projection x View)
    view_proj_mtx.setMul(mCameraProperty->GetProjectionMatrix(), view_mtx);

    sViewBlock.view_pos = mCameraProperty->GetParentNode().lock()->GetPosition();
    sViewBlock.view_proj_mtx = view_proj_mtx;

    mpViewUniformBlock->setSubDataInvalidate(&sViewBlock, 0, sizeof(ViewBlock));
    mpLightUniformBlock->setSubDataInvalidate(&sLightBlock, 0, sizeof(LightBlock));

    rio::Mtx34f nodeMtx;
    nodeMtx.makeSRT(GetParentNode().lock()->GetScale(), GetParentNode().lock()->GetRotation(), GetParentNode().lock()->GetPosition());

    mMdlModel->setModelWorldMtx(nodeMtx);

    const rio::mdl::Mesh *const meshes = mMdlModel->meshes();

    for (u32 i = 0; i < mMdlModel->numMeshes(); i++)
    {
        const rio::mdl::Mesh &mesh = meshes[i];

        if (!mesh.material())
            continue;

        const rio::mdl::Material &material = *mesh.material();
        if (!material.shader() || !material.resMaterial().isVisible())
            continue;

        material.bind();

        const UniformBlocks &uniform_block_idx = mUniformBlocks[i];

        // Set the ViewBlock index and stage
        mpViewUniformBlock->setIndex(uniform_block_idx.view_block_idx.vs, uniform_block_idx.view_block_idx.fs);
        mpViewUniformBlock->setStage(uniform_block_idx.view_block_idx.stage);
        mpViewUniformBlock->bind();

        // Get mesh world matrix
        mModelBlock[i].model_mtx = mesh.worldMtx();
        mModelBlock[i].normal_mtx.setInverseTranspose(mModelBlock[i].model_mtx);

        // Set the LightBlock index and stage
        mpLightUniformBlock->setIndex(uniform_block_idx.light_block_idx.vs, uniform_block_idx.light_block_idx.fs);
        mpLightUniformBlock->setStage(uniform_block_idx.light_block_idx.stage);
        mpLightUniformBlock->bind();

        // Update the ModelBlock uniform
        mModelUniformBlock[i].setSubDataInvalidate(&mModelBlock[i], 0, 2 * sizeof(rio::Matrix34f));
        // Bind the ModelBlock uniform
        mModelUniformBlock[i].bind();

        const rio::Shader *shader = material.shader();

        // Draw
        mesh.draw();
    }
}

YAML::Node MeshProperty::Save()
{
    YAML::Node node;

    node["Mesh"]["meshFileName"] = mMeshFileName;
    node["Mesh"]["meshKey"] = mMeshKey;

    return node;
}

void MeshProperty::CreatePropertiesMenu()
{
    // Creating ImGui UI elements logic here..
}