#include <rio.h>
#include <gfx/mdl/rio_Mesh.h>
#include <gfx/mdl/rio_Model.h>
#include <gfx/mdl/res/rio_ModelCacher.h>
#include <helpers/model/ModelNode.h>

ModelNode::ModelNode(const char *pBaseFileName, const char *pModelKey, const rio::Vector3f pScale, const rio::Vector3f pPosition)
{
    mResModel = rio::mdl::res::ModelCacher::instance()->loadModel(pBaseFileName, pModelKey);

    mModel = new rio::mdl::Model(mResModel);

    // Setting world matrix
    rio::BaseMtx34f worldMtx;

    worldMtx.v[0].w = pPosition.x;
    worldMtx.v[1].w = pPosition.y;
    worldMtx.v[2].w = pPosition.z;

    worldMtx.v[0].x = pScale.x;
    worldMtx.v[1].x = pScale.y;
    worldMtx.v[2].x = pScale.z;
}

ModelNode::~ModelNode()
{
    delete mModel;
    delete mResModel;
}
