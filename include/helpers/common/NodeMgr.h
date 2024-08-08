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

#include <helpers/properties/MiiHeadProperty.h>
#include <helpers/properties/audio/AudioProperty.h>
#include <helpers/properties/map/CameraProperty.h>
#include <helpers/properties/gfx/PrimitiveProperty.h>
#include <helpers/properties/gfx/MeshProperty.h>
#include <helpers/properties/examples/ExampleEnumProperty.h>

class NodeMgr
{
public:
    static bool createSingleton();
    static bool destorySingleton();

    static int AddNode(std::shared_ptr<Node> pNode);
    static bool DeleteNode(const int pIndex);

    bool LoadFromFile(std::string fileName);
    bool SaveToFile();

    std::vector<std::shared_ptr<Node>> mNodes;

    static inline NodeMgr *instance() { return mInstance; };
    static inline int GetNodeCount() { return mInstance ? mInstance->mNodes.size() : 0; };
    static inline void ClearAllNodes() { return mInstance->mNodes.clear(); };

    void Update();
    void Start();

    std::shared_ptr<Node> GetNodeByKey(const char *pKey);
    Node *GetNodeByID(const int ID);
    Node *GetNodeByIndex(const int pIndex);

    using PropertyCreateFunc = std::function<std::unique_ptr<Property>(std::shared_ptr<Node>)>;
    std::unordered_map<std::string, PropertyCreateFunc> mPropertyFactory = {
        {"Audio", [](std::shared_ptr<Node> node)
         { return std::make_unique<AudioProperty>(node); }},
        {"Camera", [](std::shared_ptr<Node> node)
         { return std::make_unique<CameraProperty>(node); }},
        {"Primitive", [](std::shared_ptr<Node> node)
         { return std::make_unique<PrimitiveProperty>(node); }},
        {"MiiHead", [](std::shared_ptr<Node> node)
         { return std::make_unique<MiiHeadProperty>(node); }},
        {"ExampleEnum", [](std::shared_ptr<Node> node)
         { return std::make_unique<ExampleEnumProperty>(node); }},
        {"Mesh", [](std::shared_ptr<Node> node)
         { return std::make_unique<MeshProperty>(node); }}};

private:
    static NodeMgr *mInstance;
    std::string currentFilePath = "/";

    bool mInitialized = false;
};

#endif // COMMONNODEHELPER_H