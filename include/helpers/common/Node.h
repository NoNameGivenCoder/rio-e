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

    rio::Vector3f GetScale();
    rio::Vector3f GetPosition();
    rio::Vector3f GetRotation();

    void SetScale(rio::Vector3f pScale) { transformMatrix.applyScaleLocal(pScale); };
    void SetPosition(rio::Vector3f pPos) { transformMatrix.makeT(pPos); };
    void SetRotation(rio::Vector3f pRot) { transformMatrix.makeR(pRot); };

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
};

#endif // COMMONHELPER_H