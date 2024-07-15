#include <string>
#include <helpers/common/Node.h>

class EditorMgr
{
public:
    static bool createSingleton();
    static bool destorySingleton();

    static inline EditorMgr *instance() { return mInstance; };

    static void CreateNodePropertiesMenu();

    std::string currentAssetsEditorDirectory = "/";
    std::string assetsWindowString = "Assets (/)";

    Node *selectedNode = nullptr;

private:
    static EditorMgr *mInstance;
    bool mInitialized = false;
};
