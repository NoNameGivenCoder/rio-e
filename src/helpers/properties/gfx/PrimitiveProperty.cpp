#include <helpers/properties/gfx/PrimitiveProperty.h>

YAML::Node PrimitiveProperty::Save()
{
    YAML::Node node;

    node["Primitive"]["shape"] = (int)(mShapeType);

    char buf[100];
    std::sprintf(buf, "[%f, %f, %f, %f]", mShapeColor.r, mShapeColor.g, mShapeColor.b, mShapeColor.a);
    node["Primitive"]["color"] = YAML::Load(std::string(buf));
    node["Primitive"]["propertyId"] = Property::GetPropertyID();

    return node;
}

void PrimitiveProperty::Load(YAML::Node node)
{
    if (!node["shape"])
    {
        RIO_LOG("[PRIMITIVE] Error: Missing 'shape' in YAML node.\n");
        return;
    }
    mShapeType = (ShapeType)(node["shape"].as<int>());

    if (mShapeType == SHAPE_TYPE_SPHERE)
    {
        mShapeRadius = node["sphereRadius"].as<f32>();
    }

    if (!node["color"])
    {
        RIO_LOG("[PRIMITIVE] Error: Missing 'color' in YAML node.\n");
        return;
    }

    mShapeColor = {node["color"][0].as<f32>(), node["color"][1].as<f32>(), node["color"][2].as<f32>(), node["color"][3].as<f32>()};

    Property::SetPropertyID(node["propertyId"].as<int>());
}

void PrimitiveProperty::Start() { mInitialized = true; }

void PrimitiveProperty::Update()
{
    rio::PrimitiveRenderer::instance()->begin();

    switch (mShapeType)
    {
    case SHAPE_TYPE_SPHERE:
    {
        rio::PrimitiveRenderer::instance()->drawSphere8x16(GetParentNode().lock()->GetPosition(), mShapeRadius, mShapeColor);
        break;
    }
    case SHAPE_TYPE_CUBE:
    {
        rio::PrimitiveRenderer::CubeArg cubeArg;
        cubeArg.setCenter(GetParentNode().lock()->GetPosition());
        cubeArg.setColor({1, 1, 1, 1});
        cubeArg.setSize(GetParentNode().lock()->GetScale());
        rio::PrimitiveRenderer::instance()->drawCube(cubeArg);
        break;
    }
    case SHAPE_TYPE_AXIS:
    {
        rio::Vector3f parentScale = GetParentNode().lock()->GetScale();
        f32 scale = (parentScale.x + parentScale.y + parentScale.z) / 3;
        rio::PrimitiveRenderer::instance()->drawAxis(GetParentNode().lock()->GetPosition(), scale);
        break;
    }
    case SHAPE_TYPE_CYLINDER:
    {
        rio::Vector3f parentScale = GetParentNode().lock()->GetScale();
        f32 radius = (parentScale.x + parentScale.z) / 2;
        rio::PrimitiveRenderer::instance()->drawCylinder32(GetParentNode().lock()->GetPosition(), radius, parentScale.y, mShapeColor);
        break;
    }
    }

    rio::PrimitiveRenderer::instance()->end();
}

void PrimitiveProperty::CreatePropertiesMenu()
{
}