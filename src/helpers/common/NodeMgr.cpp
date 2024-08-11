#include <math/rio_Math.h>
#include <rio.h>
#include <gfx/rio_Color.h>

#include <helpers/common/NodeMgr.h>
#include <helpers/model/LightNode.h>
#include <helpers/model/ModelNode.h>
#include <helpers/common/Node.h>
#include <helpers/properties/Property.h>

#include <gfx/rio_PrimitiveRenderer.h>

#include <cstring>
#include <vector>
#include <memory>

#include <filedevice/rio_FileDevice.h>
#include <filedevice/rio_FileDeviceMgr.h>
#include <yaml-cpp/yaml.h>

#include <helpers/editor/EditorMgr.h>

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

bool NodeMgr::DeleteNode(const int pID)
{
    mInstance->mNodes.erase(pID);
    return true;
}

int NodeMgr::AddNode(std::shared_ptr<Node> pNode)
{
    if (!pNode)
        return -1;

    mInstance->mNodes.emplace(pNode->ID, pNode);
    RIO_LOG("[NODEMGR] Added %s to NodeMgr with ID of %d.\n", pNode->nodeKey.c_str(), pNode->ID);

    return pNode->ID;
}

bool NodeMgr::LoadFromFile(std::string fileName)
{
    std::string mapFolderPath = rio::FileDeviceMgr::instance()->getMainFileDevice()->getContentNativePath() + "/map/";

    mapFolderPath.append(fileName);

    RIO_LOG("[NODEMGR] Loading YAML from %s\n", mapFolderPath.c_str());

    YAML::Node mapYaml = YAML::LoadFile(mapFolderPath);
    mInstance->currentFilePath = mapFolderPath;

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
        addedNode->ID = id;
        NodeMgr::instance()->AddNode(addedNode);

        for (YAML::const_iterator pt = node["properties"].begin(); pt != node["properties"].end(); ++pt)
        {
            std::string propertyName = pt->first.as<std::string>();
            YAML::Node propertyNode = pt->second;

            RIO_LOG("[NODEMGR] Loading Property: %s..\n", propertyName.c_str());

            auto fp = mPropertyFactory.find(propertyName);
            if (fp != mPropertyFactory.end())
            {
                auto property = fp->second(addedNode);
                property->Load(propertyNode);

                if (propertyName == "Camera")
                {
                    mCamera = static_cast<CameraProperty *>(property.get());
                    RIO_LOG("[NODEMGR] Found CameraProperty!\n");
                }

                addedNode->AddProperty(std::move(property));

                RIO_LOG("[NODEMGR] Added Property: %s\n", propertyName.c_str());
            }
            else
            {
                RIO_LOG("[NODEMGR] Unknown Property: %s\n", propertyName.c_str());
            }
        }
    }

    return true;
}

bool NodeMgr::SaveToFile()
{
    YAML::Emitter outYaml;

    outYaml << YAML::BeginMap;
    outYaml << YAML::Key << "nodes" << YAML::BeginMap;

    for (auto &it : mInstance->mNodes)
    {
        std::shared_ptr<Node> node = it.second;

        outYaml << YAML::Key << node->ID << YAML::BeginMap;
        outYaml << YAML::Key << "name" << YAML::Value << node->nodeKey;

        outYaml << YAML::Key << "transform" << YAML::BeginMap;

        outYaml << YAML::Key << "position" << YAML::BeginMap;
        outYaml << YAML::Key << "x" << YAML::Value << node->GetPosition().x;
        outYaml << YAML::Key << "y" << YAML::Value << node->GetPosition().y;
        outYaml << YAML::Key << "z" << YAML::Value << node->GetPosition().z << YAML::EndMap;

        outYaml << YAML::Key << "rotation" << YAML::BeginMap;
        outYaml << YAML::Key << "x" << YAML::Value << node->GetRotation().x;
        outYaml << YAML::Key << "y" << YAML::Value << node->GetRotation().y;
        outYaml << YAML::Key << "z" << YAML::Value << node->GetRotation().z << YAML::EndMap;

        outYaml << YAML::Key << "scale" << YAML::BeginMap;
        outYaml << YAML::Key << "x" << YAML::Value << node->GetScale().x;
        outYaml << YAML::Key << "y" << YAML::Value << node->GetScale().y;
        outYaml << YAML::Key << "z" << YAML::Value << node->GetScale().z << YAML::EndMap << YAML::EndMap;

        outYaml << YAML::Key << "properties" << YAML::BeginMap;

        for (auto &property : node->properties)
        {
            YAML::Node propertyNode = property->Save();
            outYaml << YAML::Key << propertyNode.begin()->first << YAML::Value << propertyNode.begin()->second;
        }

        outYaml << YAML::EndMap << YAML::EndMap;
    }

    RIO_LOG("%s\n", mInstance->currentFilePath.c_str());

    rio::FileDevice *fileDevice = rio::FileDeviceMgr::instance()->getNativeFileDevice();
    rio::FileHandle fileHandle;
    fileDevice->open(&fileHandle, mInstance->currentFilePath, rio::FileDevice::FILE_OPEN_FLAG_WRITE);
    fileDevice->write(&fileHandle, (u8 *)(outYaml.c_str()), strlen(outYaml.c_str()));
    fileDevice->close(&fileHandle);

    return true;
}

void NodeMgr::Start()
{
    rio::PrimitiveRenderer::instance()->begin();

    for (const auto &it : mNodes)
    {
        std::shared_ptr<Node> node = it.second;
        for (auto &property : node->properties)
        {
            property->Start();
        }
    }

    rio::PrimitiveRenderer::instance()->end();
}

void NodeMgr::Update()
{
    for (const auto &it : mNodes)
    {
        std::shared_ptr<Node> node = it.second;
        for (auto &property : node->properties)
        {
            EditorMgr::instance()->BindRenderBuffer();
            property->Update();
            EditorMgr::instance()->UnbindRenderBuffer();
        }
    }
}