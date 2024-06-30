#include <rio.h>
#include <gfx/mdl/rio_Mesh.h>
#include <gfx/mdl/rio_Model.h>

class ModelNode
{
public:
    ModelNode(const char *pModelPath, const char *pModelKey, const rio::Vector3f pScale = {1, 1, 1}, const rio::Vector3f pPosition = {0, 0, 0});
    ~ModelNode();

private:
    rio::mdl::Model *mModel;
    rio::mdl::Mesh *mMesh;

    rio::mdl::res::Mesh *mResMesh;
    rio::mdl::res::Model *mResModel;
};