#ifndef COMMONNODEHELPER_H
#define COMMONNODEHELPER_H

#include <rio.h>
#include <math/rio_Matrix.h>
#include <helpers/common/Node.h>
#include <vector>
#include <memory>
#include <string>

class NodeMgr
{
public:
    static bool createSingleton();
    static bool destorySingleton();

    static int AddNode(std::shared_ptr<Node> pNode);
    static bool DeleteNode(const int pIndex);

    static bool LoadFromFile(std::string fileName);

    std::vector<std::shared_ptr<Node>> mNodes;

    static inline NodeMgr *instance() { return mInstance; };
    static inline int GetNodeCount() { return mInstance ? mInstance->mNodes.size() : 0; };
    static inline void ClearAllNodes() { return mInstance->mNodes.clear(); };

    void Update();

    Node *GetNodeByKey(const char *pKey);
    Node *GetNodeByID(const int ID);
    Node *GetNodeByIndex(const int pIndex);

    template <typename T>
    std::vector<T *> GetNodesByType()
    {
        std::vector<T *> result;

        for (const auto &node : mNodes)
        {
            if (T *typedNode = dynamic_cast<T *>(node.get()))
            {
                // RIO_LOG("%s\n", node.get()->nodeKey);
                result.push_back(typedNode);
            }
        }

        return result;
    }

private:
    static NodeMgr *mInstance;

    bool mInitialized = false;
};

#endif // COMMONNODEHELPER_H