#include <helpers/properties/examples/ExampleEnumProperty.h>

// For editor usage we use a library called ImGui.
#include <imgui.h>

void ExampleEnumProperty::Load(YAML::Node node)
{
    // Loading in our example enum.
    mExampleEnum = (ExampleEnum)(node["exampleEnum"].as<int>());

    // Loading in our property ID.
    SetPropertyID(node["propertyId"].as<int>());
}

void ExampleEnumProperty::Start()
{
    // Start logic here..
}

void ExampleEnumProperty::Update()
{
    // Update loop logic here..
}

YAML::Node ExampleEnumProperty::Save()
{
    YAML::Node node;

    // Saving the example enum.
    node["ExampleEnum"]["exampleEnum"] = (int)(mExampleEnum);

    // Saving our property ID.
    node["ExampleEnum"]["propertyId"] = GetPropertyID();

    return node;
}

void ExampleEnumProperty::CreatePropertiesMenu()
{
    // Set the ID of the collapsing header.
    ImGui::PushID(GetPropertyID());

    if (ImGui::CollapsingHeader("Example Enum"))
    {
        ImGui::PopID();

        // Creating a unique ID for our dropdown.
        std::string comboId = "EnumPropertyCombo(" + std::to_string(GetPropertyID()) + ")";
        ImGui::PushID(comboId.c_str());

        if (ImGui::BeginCombo("", ExampleEnumInfo[(int)(mExampleEnum)].name))
        {
            // Create our selectables
            for (int i = 0; i < 2; ++i)
            {
                // Calculate if our selectable should appear as selected.
                bool isSelected = (mExampleEnum == (ExampleEnum)(i));

                if (ImGui::Selectable(ExampleEnumInfo[i].name, isSelected))
                {
                    // Finally, if our selectable is clicked, we change our class member enum to the selected value.
                    mExampleEnum = (ExampleEnum)(ExampleEnumInfo[i].value);
                }

                // If our selectable is selected, we set the default focus to it.
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
        }
    };
}