#ifndef COMMONNODEHELPER_H
#define COMMONNODEHELPER_H

#include <rio.h>
#include <math/rio_Matrix.h>
#include <helpers/common/Node.h>
#include <vector>
#include <memory>
#include <string>

#include <unordered_map>
#include <functional>

#include <helpers/editor/EditorMgr.h>

#include <helpers/properties/MiiHeadProperty.h>
#include <helpers/properties/audio/AudioProperty.h>
#include <helpers/properties/map/CameraProperty.h>
#include <helpers/properties/gfx/PrimitiveProperty.h>
#include <helpers/properties/gfx/MeshProperty.h>
#include <helpers/properties/examples/ExampleEnumProperty.h>

class NodeMgr
{
    friend class EditorMgr;

public:
    using PropertyCreateFunc = std::function<std::unique_ptr<Property>(std::shared_ptr<Node>)>;

    static bool createSingleton();
    static bool destorySingleton();

    static int AddNode(std::shared_ptr<Node> pNode);
    static bool DeleteNode(const int pID);

    bool LoadFromFile(std::string fileName);

    bool SaveToFile();

    static inline NodeMgr *instance() { return mInstance; };
    static inline int GetNodeCount() { return mInstance->mNodes.size(); };
    static inline void ClearAllNodes() { return mInstance->mNodes.clear(); };

    /// @brief Finds a node with a certain ID.
    /// @param pID ID to look for.
    /// @return Found node pointer.
    static inline std::shared_ptr<Node> GetNodeByID(int pID) { return mInstance->mNodes.at(pID); };

    /// @brief Should be ran every frame. Updates all properties throughout the scene.
    void Update();

    /// @brief Should be ran in a task's `prepare_()` method. Starts all properties throughout the scene.
    void Start();

    /// @brief Gets the global `CameraProperty`.
    /// @return Pointer to global `CameraProperty`
    static inline CameraProperty *GetGlobalCamera() { return mInstance->mCamera; };

    /// @brief Gets the global `SunProperty`.
    /// @return Pointer to global `SunProperty`.
    static inline CameraProperty *GetGlobalSun() { return mInstance->mCamera; };

    /// @brief Registers a property into the property map.
    /// @param name Name that'll be used for property creation (Map YAML, `CreateProperty()`, etc.)
    /// @param func Property create function.
    static inline void RegisterProperty(const std::string name, PropertyCreateFunc func)
    {
        mInstance->mPropertyMap[name] = func;
    };

    /// @brief Creates a property and sets ownership to `node`
    /// @param name Property global name to create.
    /// @param node Parent node. Created property will be attatched to it.
    /// @return Created property pointer.
    static inline std::unique_ptr<Property> CreateProperty(const std::string name, std::shared_ptr<Node> node)
    {
        auto it = mInstance->mPropertyMap.find(name);
        if (it != mInstance->mPropertyMap.end())
        {
            return it->second(node);
        }

        RIO_LOG("[NODEMGR] No property by the name: %s exists.", name.c_str());
        return nullptr;
    }

    /// @brief Returns all property names for use.
    /// @return Vector containing all property names for use.
    std::vector<std::string> GetAvailablePropertyNames() const
    {
        std::vector<std::string> retVal;

        for (const auto &val : mPropertyMap)
        {
            retVal.emplace_back(val.first);
        }

        return retVal;
    }

private:
    static NodeMgr *mInstance;
    std::string currentFilePath = "/";
    CameraProperty *mCamera = nullptr;

    bool mInitialized = false;

    std::unordered_map<int, std::shared_ptr<Node>> mNodes = {};
    std::unordered_map<std::string, PropertyCreateFunc> mPropertyMap = {};
};

#endif // COMMONNODEHELPER_H