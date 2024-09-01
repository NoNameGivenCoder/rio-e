#include "rio-e/EditorMgr.h"

namespace rioe
{
    EditorMgr* EditorMgr::mInstance = nullptr;

    bool EditorMgr::createSingleton()
    {
        if (mInstance)
            return false;

        mInstance = new EditorMgr();

        return true;
    }

    bool EditorMgr::destorySingleton()
    {
        if (!mInstance)
            return false;

        delete mInstance;
        mInstance = nullptr;

        return true;
    }

    void EditorMgr::InitializeProject(EditorTypes::Project project)
    {
        mCurrentProject = project;
    }
}