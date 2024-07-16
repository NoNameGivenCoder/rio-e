#include <helpers/common/Node.h>
#include <math/rio_Math.h>
#include <helpers/common/NodeMgr.h>

Node::Node(std::string pNodeKey, rio::Vector3f pPos, rio::Vector3f pRot, rio::Vector3f pScale)
{
    nodeKey = pNodeKey;
    transformMatrix.makeSRT(pScale, pRot, pPos);
    ID = NodeMgr::instance()->GetNodeCount() + 1;

    RIO_LOG("[NODE] New node created with key: %s.\n", nodeKey.c_str());
    NodeMgr::instance()->AddNode(this);
};

rio::Vector3f Node::GetScale()
{
    rio::Vector3f scale;

    // Extract the basis vectors
    const rio::Vector3f &v1 = reinterpret_cast<const rio::Vector3f &>(transformMatrix.m[0][0]);
    const rio::Vector3f &v2 = reinterpret_cast<const rio::Vector3f &>(transformMatrix.m[1][0]);
    const rio::Vector3f &v3 = reinterpret_cast<const rio::Vector3f &>(transformMatrix.m[2][0]);

    // Compute the length (norm) of each basis vector to get the scale factors
    scale.x = std::sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
    scale.y = std::sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z);
    scale.z = std::sqrt(v3.x * v3.x + v3.y * v3.y + v3.z * v3.z);

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

int Node::AddProperty(Property *pProperty)
{
    properties.emplace_back(pProperty);
    pProperty->parentNode = this;
    return 0;
}