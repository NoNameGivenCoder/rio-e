#include <math/rio_Math.h>
#include <rio.h>
#include <gfx/rio_Color.h>

#include <helpers/common/NodeMgr.h>
#include <helpers/model/LightNode.h>
#include <helpers/model/ModelNode.h>
#include <helpers/common/Node.h>
#include <helpers/properties/Property.h>
#include <helpers/properties/audio/AudioProperty.h>
#include <helpers/properties/map/CameraProperty.h>

#include <cstring>
#include <vector>
#include <memory>

#include <filedevice/rio_FileDevice.h>
#include <filedevice/rio_FileDeviceMgr.h>
#include <yaml-cpp/yaml.h>

NodeMgr *NodeMgr::mInstance = nullptr;

bool NodeMgr::createSingleton()
{
    if (mInstance)
        return false;

    mInstance = new NodeMgr();
    mInstance->mInitialized = true;

    if (!mInstance->mInitialized)
    {
        delete mInstance;
        mInstance = nullptr;
        return false;
    }

    return true;
}

bool NodeMgr::destorySingleton()
{
    if (!mInstance)
        return false;

    mInstance->ClearAllNodes();
    delete mInstance;
    mInstance = nullptr;

    return true;
}

bool NodeMgr::DeleteNode(const int pIndex)
{
    if (pIndex < 0 || pIndex > mInstance->mNodes.size())
        return false;

    mInstance->mNodes.erase(mInstance->mNodes.begin() + pIndex);

    return true;
}

int NodeMgr::AddNode(std::shared_ptr<Node> pNode)
{
    if (!pNode)
        return -1;

    mInstance->mNodes.push_back(pNode);
    RIO_LOG("[NODEMGR] Added %s to NodeMgr.\n", pNode->nodeKey.c_str());

    return mInstance->mNodes.size() - 1;
}

Node *NodeMgr::GetNodeByIndex(const int pIndex)
{
    return mInstance->mNodes.at(pIndex).get();
}

Node *NodeMgr::GetNodeByKey(const char *pKey)
{
    for (const auto &node : mNodes)
    {
        if (strcmp(node.get()->nodeKey.c_str(), pKey) == 0)
            return node.get();
    }

    return nullptr;
}

Node *NodeMgr::GetNodeByID(const int ID)
{
    for (const auto &node : mNodes)
    {
        if (node.get()->ID == ID)
            return node.get();
    }

    return nullptr;
}

bool NodeMgr::LoadFromFile(std::string fileName)
{
    std::string mapFolderPath = rio::FileDeviceMgr::instance()->getMainFileDevice()->getContentNativePath() + "/map/";

    mapFolderPath.append(fileName);

    RIO_LOG("[NODEMGR] Loading YAML from %s\n", mapFolderPath.c_str());

    YAML::Node mapYaml = YAML::LoadFile(mapFolderPath);

    if (!mapYaml["nodes"])
        return false;

    YAML::Node nodes = mapYaml["nodes"];

    for (YAML::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        int id = it->first.as<int>();

        YAML::Node node = it->second;

        std::string nodeName = node["name"].as<std::string>();

        rio::Vector3f nodePosition, nodeRotation, nodeScale;
        nodePosition = {node["transform"]["position"]["x"].as<f32>(), node["transform"]["position"]["y"].as<f32>(), node["transform"]["position"]["z"].as<f32>()};
        nodeRotation = {node["transform"]["rotation"]["x"].as<f32>(), node["transform"]["rotation"]["y"].as<f32>(), node["transform"]["rotation"]["z"].as<f32>()};
        nodeScale = {node["transform"]["scale"]["x"].as<f32>(), node["transform"]["scale"]["y"].as<f32>(), node["transform"]["scale"]["z"].as<f32>()};

        auto addedNode = std::make_shared<Node>(nodeName, nodePosition, nodeRotation, nodeScale);
        NodeMgr::instance()->AddNode(addedNode);

        for (YAML::const_iterator pt = node["properties"].begin(); pt != node["properties"].end(); ++pt)
        {
            std::string propertyName = pt->first.as<std::string>();
            YAML::Node propertyNode = pt->second;

            RIO_LOG("[NODEMGR] Loading Property: %s..\n", propertyName.c_str());

            if (propertyName == "Audio")
            {
                auto audioProperty = std::make_unique<AudioProperty>(addedNode);
                audioProperty->Load(propertyNode);
                addedNode->AddProperty(std::move(audioProperty));
            }

            if (propertyName == "Camera")
            {
                auto cameraProperty = std::make_unique<CameraProperty>(addedNode);
                cameraProperty->Load(propertyNode);
                addedNode->AddProperty(std::move(cameraProperty));
            }

            RIO_LOG("[NODEMGR] Added Property: %s\n", propertyName.c_str());
        }
    }

    return true;
}

void NodeMgr::Update()
{
    for (auto &node : mNodes)
    {
        for (auto &property : node->properties)
        {
            property->Update();
        }
    }
}