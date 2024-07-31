#include <helpers/properties/examples/ExampleProperty.h>

void ExampleProperty::Load(YAML::Node node)
{
    // Loading from YAML logic here..
}

void ExampleProperty::Start()
{
    // Start logic here..
}

void ExampleProperty::Update()
{
    // Update loop logic here..
}

YAML::Node ExampleProperty::Save()
{
    YAML::Node node;

    // Saving into YAML node logic here..

    return node;
}

void ExampleProperty::CreatePropertiesMenu()
{
    // Creating ImGui UI elements logic here..
}