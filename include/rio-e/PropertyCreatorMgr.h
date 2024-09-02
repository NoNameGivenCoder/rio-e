#pragma once

#include <memory>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>

#include "yaml-cpp/yaml.h"
#include "rio.h"

namespace rioe
{
    class Node;
    class Property
    {
    public:
        Property() {};
        virtual ~Property() = default;

        virtual void Start() = 0;
        virtual void Update() = 0;
        virtual void CreatePropertiesMenu() = 0;

        inline std::weak_ptr<Node> GetParentNode() const { return parentNode; };

        virtual YAML::Node Save() = 0;
        virtual void Load(YAML::Node node) = 0;
    private:
        std::weak_ptr<Node> parentNode;
    };

    class PropertyCreatorMgr
    {
    public:
        using CreatorFunc = std::function<std::unique_ptr<Property>()>;

        static bool createSingleton();
        static bool destorySingleton();

        static inline PropertyCreatorMgr* instance() { return mInstance; };

        inline void RegisterProperty(const std::string& type, CreatorFunc creator)
        {
            RIO_LOG("[PropertyCreatorMgr] Registered property: %s\n", type.c_str());
            mCreators[type] = creator;
        }

        std::unique_ptr<Property> CreateProperty(const std::string& type);

        std::vector<std::string> GetAvaliableProperties();

    private:
        std::unordered_map<std::string, CreatorFunc> mCreators;
        static PropertyCreatorMgr* mInstance;
    };
}