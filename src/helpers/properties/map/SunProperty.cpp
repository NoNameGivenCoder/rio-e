#include <helpers/properties/map/SunProperty.h>

void SunProperty::Load(YAML::Node node)
{
    mSunColor.r = node["sunColor"][0].as<int>();
    mSunColor.g = node["sunColor"][1].as<int>();
    mSunColor.b = node["sunColor"][2].as<int>();
    mSunColor.a = node["sunColor"][3].as<int>();

    SetPropertyID(node["propertyId"].as<int>());
}

void SunProperty::Start()
{
    // Start logic here..
}

void SunProperty::Update()
{
    // Update loop logic here..
}

YAML::Node SunProperty::Save()
{
    YAML::Node node;

    // Saving into YAML node logic here..

    return node;
}

void SunProperty::CreatePropertiesMenu()
{
    // Creating ImGui UI elements logic here..
}