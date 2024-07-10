#include <helpers/common/Node.h>
#include <math/rio_Math.h>
#include <helpers/common/NodeMgr.h>

Node::Node(const char *pNodeKey, rio::Vector3f pPos, rio::Vector3f pRot, rio::Vector3f pScale)
{
    nodeKey = (char *)pNodeKey;
    transformMatrix.makeSRT(pScale, pRot, pPos);
    ID = NodeMgr::instance()->GetNodeCount() + 1;

    RIO_LOG("[NODE] New node created with key: %s.\n", nodeKey);
    NodeMgr::instance()->AddNode(this);
};

rio::Vector3f Node::GetScale()
{
    rio::Vector3f scale;

    const rio::Vector3f &v1 = reinterpret_cast<const rio::Vector3f &>(transformMatrix.m[0][0]);
    const rio::Vector3f &v2 = reinterpret_cast<const rio::Vector3f &>(transformMatrix.m[1][0]);
    const rio::Vector3f &v3 = reinterpret_cast<const rio::Vector3f &>(transformMatrix.m[2][0]);

    scale.x = rio::Mathf::sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
    scale.y = rio::Mathf::sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z);
    scale.z = rio::Mathf::sqrt(v3.x * v3.x + v3.y * v3.y + v3.z * v3.z);

    return scale;
}

rio::Vector3f Node::GetPosition()
{
    return {transformMatrix.m[0][3], transformMatrix.m[1][3], transformMatrix.m[2][3]};
}