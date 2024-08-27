#ifndef PRIMITIVEPROPERTY_H
#define PRIMITIVEPROPERTY_H

#include <helpers/properties/Property.h>

#include <gfx/rio_Color.h>
#include <gfx/rio_PrimitiveRenderer.h>

class PrimitiveProperty : public Property
{
public:
    enum ShapeType
    {
        SHAPE_TYPE_SPHERE = 0,
        SHAPE_TYPE_CUBE = 1,
        SHAPE_TYPE_AXIS = 2,
        SHAPE_TYPE_CYLINDER = 3
    };

    using Property::Property;

    void Load(YAML::Node node);
    YAML::Node Save() override;

    void Update() override;
    void Start() override;
    void CreatePropertiesMenu() override;

private:
    ShapeType mShapeType;
    f32 mShapeRadius;
    rio::Color4f mShapeColor;
};

#endif // PRIMITIVEPROPERTY_H