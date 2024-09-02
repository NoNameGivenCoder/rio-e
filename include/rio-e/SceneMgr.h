#pragma once

#include "rio.h"
#include "math/rio_MathTypes.h"
#include "math/rio_Matrix.h"

#include "rio-e/PropertyCreatorMgr.h"

#include "yaml-cpp/yaml.h"

#include <unordered_map>
#include <memory>
#include <filesystem>
#include <functional>

namespace rioe
{
    class TestProperty : public Property
    {
    public:
        void Start() override
        {
            // Implementation of Start
        }

        void Update() override
        {
            // Implementation of Update
        }

        void CreatePropertiesMenu() override
        {
            // Implementation of CreatePropertiesMenu
        }

        YAML::Node Save() override
        {
            // Implementation of Save
            return YAML::Node(); // Replace with actual implementation
        }

        void Load(YAML::Node node) override
        {
            // Implementation of Load
        }
    };

    class Node
    {
    public:
        int ID;

        inline rio::Vector3f GetScale() const { return mScale; };
        inline rio::Vector3f GetPosition() const { return mPosition; };
        inline rio::Vector3f GetRotation() const { return mRotation; };

        inline void SetScale(rio::Vector3f pScale)
        {
            mScale = pScale;
            return UpdateMatrix();
        };

        inline void SetPosition(rio::Vector3f pPos)
        {
            mPosition = pPos;
            return UpdateMatrix();
        };

        inline void SetRotation(rio::Vector3f pRot)
        {
            mRotation = pRot;
            return UpdateMatrix();
        };

        inline void AddProperty(std::unique_ptr<Property> property)
        {
            property->Start();
            mProperties.push_back(std::move(property));
        }

    private:
        rio::Matrix34f mTransformMatrix;
        rio::Vector3f mPosition;
        rio::Vector3f mRotation;
        rio::Vector3f mScale;

        inline void UpdateMatrix() { mTransformMatrix.makeSRT(mScale, mRotation, mPosition); };

        std::vector<std::unique_ptr<Property>> mProperties;
    };

    class SceneMgr
    {
    public:
        struct LoadArg
        {
            std::filesystem::path path;
            bool reloadScene = true;
        };
    public:
        static bool createSingleton();
        static bool destorySingleton();

        static inline SceneMgr* instance() { return mInstance; };

        void Load(LoadArg pLoadArg);

        std::shared_ptr<Node> CreateNode();
        inline std::shared_ptr<Node> GetNodeByID(int ID) { return mNodes[ID]; };
    private:
        std::unordered_map<int, std::shared_ptr<Node>> mNodes;
        static SceneMgr* mInstance;
    };
}
