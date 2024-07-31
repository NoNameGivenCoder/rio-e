#ifndef COMMONHELPER_H
#define COMMONHELPER_H

#include <rio.h>
#include <math/rio_Matrix.h>
#include <vector>
#include <memory>
#include <string>
#include <helpers/properties/Property.h>

class Property;

class Node
{
public:
    rio::Matrix34f transformMatrix;
    std::vector<std::unique_ptr<Property>> properties;
    std::string nodeKey;
    int ID;

    virtual ~Node() { properties.clear(); };
    Node(std::string pNodeKey, rio::Vector3f pPos, rio::Vector3f pRot, rio::Vector3f pScale);

    inline rio::Vector3f GetScale() { return mScale; };
    inline rio::Vector3f GetPosition() { return mPosition; };
    inline rio::Vector3f GetRotation() { return mRotation; };

    inline void SetScale(rio::Vector3f pScale)
    {
        mScale = pScale;
        return UpdateMatrix();
    };

    inline void SetPosition(rio::Vector3f pPos)
    {
        mPosition = pPos;
        return UpdateMatrix();
    };

    inline void SetRotation(rio::Vector3f pRot)
    {
        mRotation = pRot;
        return UpdateMatrix();
    };

    void CreateNodeProperties();

    bool isEditorSelected = false;

    bool AddProperty(std::unique_ptr<Property> pProperty);

    template <typename T>
    std::vector<T *> GetProperty()
    {
        std::vector<T *> result;

        for (const auto &property : properties)
        {
            if (T *propertyFound = dynamic_cast<T *>(property.get()))
            {
                result.push_back(propertyFound);
            }
        }

        return result;
    }

private:
    rio::Vector3f mPosition;
    rio::Vector3f mRotation;
    rio::Vector3f mScale;

    inline void UpdateMatrix() { transformMatrix.makeSRT(mScale, mRotation, mPosition); };
};

#endif // COMMONHELPER_H