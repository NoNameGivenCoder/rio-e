#ifndef COMMONPROPERTYHELPER_H
#define COMMONPROPERTYHELPER_H

#include <helpers/common/Node.h>
#include <yaml-cpp/yaml.h>

class Node;

class Property
{
public:
    Property(std::shared_ptr<Node> pParentNode) : parentNode(pParentNode) {};
    virtual ~Property() = default;

    virtual void Start() = 0;
    virtual void Update() = 0;
    virtual void CreatePropertiesMenu() = 0;

    virtual YAML::Node Save() = 0;
    virtual void Load(YAML::Node node) = 0;

    inline std::weak_ptr<Node> GetParentNode() const { return parentNode; };
    inline int GetPropertyID() const { return propertyId; };

    inline void SetPropertyID(int pPropertyId) { propertyId = pPropertyId; };
    inline void SetLoggingString(std::string pLoggingString) { loggingString = pLoggingString; };

    bool mInitialized = false;

private:
    std::string loggingString = "PROPERTY";
    std::weak_ptr<Node> parentNode;
    int propertyId = 0;
};

#endif // COMMONPROPERTYHELPER_H