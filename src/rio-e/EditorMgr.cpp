#include "rio-e/EditorMgr.h"
#include "rio-e/SceneMgr.h"

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

        RIO_LOG("[EditorMgr] Initializing %s..\n", project.projectName.c_str());
        RIO_LOG("[EditorMgr] Checking file path %s..\n", project.filePath.string().c_str());

        if (!std::filesystem::exists(project.filePath))
        {
            RIO_LOG("[EditorMgr] Path doesn't exist anymore!\n");
            return;
        }

        RIO_LOG("[EditorMgr] Initializing default scene %s..\n", project.defaultScene.c_str());
        
        std::string defaultSceneString = project.filePath.string() + "/build/fs/content/map/" + project.defaultScene;

        if (!std::filesystem::exists(defaultSceneString))
        {
            RIO_LOG("[EditorMgr] Map file: %s doesn't exist!\n", std::string(project.filePath.string() + "/build/fs/content/map/" + project.defaultScene).c_str());
            return;
        }

        SceneMgr::instance()->Load({ defaultSceneString, true });
    }
}