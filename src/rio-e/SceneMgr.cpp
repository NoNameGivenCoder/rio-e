#include "rio-e/SceneMgr.h"
#include "rio-e/PropertyCreatorMgr.h"

#include "gpu/rio_Drawer.h"

namespace rioe
{
    SceneMgr* SceneMgr::mInstance = nullptr;

    bool SceneMgr::createSingleton()
    {
        if (mInstance)
            return false;

        mInstance = new SceneMgr();

        return true;
    }

    bool SceneMgr::destorySingleton()
    {
        if (!mInstance)
            return false;

        delete mInstance;
        mInstance = nullptr;

        return true;
    }

    std::shared_ptr<Node> SceneMgr::CreateNode()
    {
        auto node = std::make_shared<Node>();
        node->ID = mNodes.size();

        mNodes.emplace(node->ID, node);

        return node;
    }

    void SceneMgr::Load(LoadArg pLoadArg)
    {
        PropertyCreatorMgr::instance()->RegisterProperty("TestPropertyYAMLName", []() {
            return std::make_unique<TestProperty>();
        });

        RIO_LOG("[SceneMgr] Loading from %s..\n", pLoadArg.path.string().c_str());

        YAML::Node YAMLNodes = YAML::LoadFile(pLoadArg.path.string());

        for (YAML::const_iterator it = YAMLNodes["nodes"].begin(); it != YAMLNodes["nodes"].end(); ++it)
        {
            int id = it->first.as<int>();

            YAML::Node node = it->second;

            auto createdNode = SceneMgr::instance()->CreateNode();
            createdNode->SetPosition({ node["transform"]["position"]["x"].as<f32>(), node["transform"]["position"]["y"].as<f32>(), node["transform"]["position"]["z"].as<f32>() });
            createdNode->SetRotation({ node["transform"]["rotation"]["x"].as<f32>(), node["transform"]["rotation"]["y"].as<f32>(), node["transform"]["rotation"]["z"].as<f32>() });
            createdNode->SetScale({ node["transform"]["scale"]["x"].as<f32>(), node["transform"]["scale"]["y"].as<f32>(), node["transform"]["scale"]["z"].as<f32>() });
            createdNode->ID = id;

            for (YAML::const_iterator pt = node["properties"].begin(); pt != node["properties"].end(); ++pt)
            {
                std::string propertyType = pt->first.as<std::string>();
                auto property = PropertyCreatorMgr::instance()->CreateProperty(propertyType);

                if (property)
                {
                    property->Load(pt->second);
                    createdNode->AddProperty(std::move(property));
                }
            }

            RIO_LOG("[SceneMgr] Created new node %d.\n", createdNode->ID);
        }
    }
}