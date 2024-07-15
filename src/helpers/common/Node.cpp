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

    scale.x = reinterpret_cast<rio::Vector3f &>(transformMatrix.v[0].x);
    scale.y = reinterpret_cast<rio::Vector3f &>(transformMatrix.v[1].x);
    scale.z = reinterpret_cast<rio::Vector3f &>(transformMatrix.v[2].x);

    return scale;
}

rio::Vector3f Node::GetPosition()
{
    return {transformMatrix.m[0][3], transformMatrix.m[1][3], transformMatrix.m[2][3]};
}

rio::Vector3f Node::GetRotation()
{
    float rotationMatrix[3][3];

    for (int i = 0; i < 3; ++i)
    {
        rotationMatrix[0][i] = transformMatrix.m[0][i] / Node::GetScale().x;
        rotationMatrix[1][i] = transformMatrix.m[1][i] / Node::GetScale().y;
        rotationMatrix[2][i] = transformMatrix.m[2][i] / Node::GetScale().z;
    }

    rio::Vector3f rotation;

    rotation.y = std::asin(-rotationMatrix[0][2]);

    if (std::cos(rotation.y) != 0)
    {
        rotation.x = std::atan2(rotationMatrix[1][2], rotationMatrix[2][2]);
        rotation.z = std::atan2(rotationMatrix[0][1], rotationMatrix[0][0]);
    }
    else
    {
        rotation.x = std::atan2(-rotationMatrix[2][0], rotationMatrix[1][1]);
        rotation.z = 0;
    }

    return rotation;
}