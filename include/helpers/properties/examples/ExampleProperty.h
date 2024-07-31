#include <helpers/properties/Property.h>

class ExampleProperty : public Property
{
public:
    // All class members here will be accessible from any other properties within the task.

    using Property::Property;

    // Called when the task is loading from YAML. Used for loading values into members of your property class.
    void Load(YAML::Node node) override;

    // Called when task starts. Used for initializing values, and preparing for rendering or controlling.
    void Start() override;

    // Called every frame.
    void Update() override;

    // Editor function. Do not use within normal gameplay.
    // Called when a task is saving. Used for saving values into a YAML node.
    YAML::Node Save() override;

    // Editor function. Do not use within normal gameplay.
    // Called when a property is selected within the editor. Used for creating ImGui UI to change default property values.
    void CreatePropertiesMenu() override;

private:
    // Private class members for use within your property.
};