#ifndef COMMONPROPERTYHELPER_H
#define COMMONPROPERTYHELPER_H

#include <helpers/common/Node.h>
#include <yaml-cpp/yaml.h>

class Node;

class Property
{
public:
    Property(std::shared_ptr<Node> pParentNode) : parentNode(pParentNode){};
    virtual ~Property() = default;

    virtual void Start() {};
    virtual void Update() {};

    virtual YAML::Node Save() {};
    virtual void Load(YAML::Node node) {};

    inline std::weak_ptr<Node> GetParentNode() const { return parentNode; };
    inline int GetPropertyID() const { return propertyId; };

    inline void SetPropertyID(int pPropertyId) { propertyId = pPropertyId; };
    inline void SetLoggingString(std::string pLoggingString) { loggingString = pLoggingString; };

private:
    std::string loggingString = "PROPERTY";
    std::weak_ptr<Node> parentNode;
    int propertyId;
};

#endif // COMMONPROPERTYHELPER_H