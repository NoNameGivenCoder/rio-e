#ifndef COMMONPROPERTYHELPER_H
#define COMMONPROPERTYHELPER_H

#include <helpers/common/Node.h>
#include <yaml-cpp/yaml.h>

class Node;

class Property
{
public:
    Node *parentNode;
    Property(Node *pParentNode);
    virtual ~Property() = default;
};

#endif // COMMONPROPERTYHELPER_H