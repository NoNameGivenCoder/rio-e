#ifndef COMMONHELPER_H
#define COMMONHELPER_H

#include <rio.h>
#include <math/rio_Matrix.h>

class Node
{
public:
    rio::Matrix34f transformMatrix;
    char *nodeKey;
    int ID;

    virtual ~Node() = default;
    Node(const char *pNodeKey, rio::Vector3f pPos, rio::Vector3f pRot, rio::Vector3f pScale);

    rio::Vector3f GetScale();
    rio::Vector3f GetPosition();

    void SetScale(rio::Vector3f pScale) { transformMatrix.makeS(pScale); };
    void SetPosition(rio::Vector3f pPos) { transformMatrix.makeT(pPos); };
    void SetRotation(rio::Vector3f pRot) { transformMatrix.makeR(pRot); };
};

#endif // COMMONHELPER_H