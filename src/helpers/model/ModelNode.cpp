#include <gpu/rio_Drawer.h>
#include <misc/rio_MemUtil.h>

#include <helpers/model/ModelNode.h>
#include <helpers/common/NodeMgr.h>

#include <helpers/model/LightNode.h>
#include <helpers/common/CameraNode.h>

#include <vector>
#include <memory>

__attribute__((aligned(rio::Drawer::cUniformBlockAlignment))) ModelNode::ViewBlock ModelNode::sViewBlock;
__attribute__((aligned(rio::Drawer::cUniformBlockAlignment))) ModelNode::LightBlock ModelNode::sLightBlock;

ModelNode::ModelNode(rio::mdl::res::Model *res_mdl, const char *view_block_name, const char *light_block_name, const char *model_block_name)
    : rio::mdl::Model(res_mdl)
{
    u32 num_meshes = numMeshes();

    // Create view uniform block instance
    mpViewUniformBlock = new rio::UniformBlock();
    mpViewUniformBlock->setData(&sViewBlock, sizeof(ModelNode::ViewBlock));

    LightNode *mLight = NodeMgr::instance()->GetNodesByType<LightNode>().at(0);

    rio::Color4f lightColor = mLight->GetLightColor();
    sLightBlock.light_color = {lightColor.r, lightColor.g, lightColor.b};
    sLightBlock.light_pos = mLight->GetPosition();

    mpLightUniformBlock = new rio::UniformBlock();
    mpLightUniformBlock->setDataInvalidate(&sLightBlock, sizeof(ModelNode::LightBlock));

    mModelUniformBlock = (rio::UniformBlock *)rio::MemUtil::alloc(num_meshes * sizeof(rio::UniformBlock), 4);
    mModelBlock = (ModelBlock *)rio::MemUtil::alloc(num_meshes * sizeof(ModelBlock), rio::Drawer::cUniformBlockAlignment);
    mUniformBlocks = (UniformBlocks *)rio::MemUtil::alloc(num_meshes * sizeof(UniformBlocks), 4);

    for (u32 i = 0; i < num_meshes; i++)
    {
        const rio::mdl::Mesh *p_mesh = &(meshes()[i]);
        const rio::mdl::Material *p_material = p_mesh->material();

        ShaderLocation view_block_idx;
        ShaderLocation light_block_idx;
        ShaderLocation model_block_idx;

        if (p_material)
        {
            const rio::Shader *p_shader = p_material->shader();
            RIO_ASSERT(p_shader);

            view_block_idx.vs = p_shader->getVertexUniformBlockIndex(view_block_name);
            view_block_idx.fs = p_shader->getFragmentUniformBlockIndex(view_block_name);
            view_block_idx.findStage();

            light_block_idx.vs = p_shader->getVertexUniformBlockIndex(light_block_name);
            light_block_idx.fs = p_shader->getFragmentUniformBlockIndex(light_block_name);
            light_block_idx.findStage();

            model_block_idx.vs = p_shader->getVertexUniformBlockIndex(model_block_name);
            model_block_idx.fs = p_shader->getFragmentUniformBlockIndex(model_block_name);
            model_block_idx.findStage();
        }

        new (&mModelUniformBlock[i]) rio::UniformBlock(model_block_idx.stage, model_block_idx.vs, model_block_idx.fs);
        mModelUniformBlock[i].setData(&mModelBlock[i], sizeof(ModelBlock));

        new (&mUniformBlocks[i]) UniformBlocks(view_block_idx, light_block_idx);
    }
}

ModelNode::~ModelNode()
{
    rio::MemUtil::free(mModelBlock);

    for (u32 i = 0; i < numMeshes(); i++)
        mModelUniformBlock[i].~UniformBlock();

    rio::MemUtil::free(mModelUniformBlock);

    delete mpViewUniformBlock;
}

void ModelNode::Draw() const
{
    CameraNode *mCamera = NodeMgr::instance()->GetNodesByType<CameraNode>().at(0);

    if (!mCamera)
        return;

    rio::Matrix34f view_mtx;
    mCamera->mCamera.getMatrix(&view_mtx);

    // Calculate view-projection matrix (Projection x View)
    rio::Matrix44f view_proj_mtx;
    view_proj_mtx.setMul(mCamera->mProjMtx, view_mtx);

    sViewBlock.view_pos = mCamera->GetPosition();
    sViewBlock.view_proj_mtx = view_proj_mtx;

    rio::RenderState render_state;
    render_state.apply();

    LightNode *mLight = NodeMgr::instance()->GetNodesByType<LightNode>().at(0);

    sLightBlock.light_color = {mLight->GetLightColor().r, mLight->GetLightColor().g, mLight->GetLightColor().b};
    sLightBlock.light_pos = mLight->GetPosition();

    mpViewUniformBlock->setSubDataInvalidate(&sViewBlock, 0, sizeof(ModelNode::ViewBlock));
    mpLightUniformBlock->setSubDataInvalidate(&sLightBlock, 0, sizeof(ModelNode::LightBlock));

    const rio::mdl::Mesh *const meshes = this->meshes();

    // Render each mesh in order
    for (u32 i = 0; i < numMeshes(); i++)
    {
        // Get the mesh
        const rio::mdl::Mesh &mesh = meshes[i];
        if (!mesh.material())
            continue;

        // Get the material
        const rio::mdl::Material &material = *mesh.material();
        if (!material.shader() || !material.resMaterial().isVisible())
            continue;

        // Bind the material
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