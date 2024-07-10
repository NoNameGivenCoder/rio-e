#include <math/rio_Math.h>
#include <rio.h>
#include <gfx/rio_Color.h>

#include <helpers/common/NodeMgr.h>
#include <helpers/common/CameraNode.h>
#include <helpers/model/LightNode.h>
#include <helpers/model/ModelNode.h>

#include <cstring>
#include <vector>
#include <memory>

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

int NodeMgr::AddNode(Node *pNode)
{
    if (!pNode)
        return -1;

    mInstance->mNodes.emplace_back(std::unique_ptr<Node>(pNode));
    RIO_LOG("[NODEMGR] Added %s to NodeMgr.\n", pNode->nodeKey);

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
        if (strcmp(node.get()->nodeKey, pKey) == 0)
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