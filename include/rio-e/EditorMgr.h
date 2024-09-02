#pragma once

#include "rio-e/EditorTypes.h"

namespace rioe
{
	class EditorMgr
	{
	public:
		enum UIState
		{
			UI_STATE_PROJECTS,
			UI_STATE_INITIALIZING_EDITOR,
			UI_STATE_EDITOR
		};
	public:
		static bool createSingleton();
		static bool destorySingleton();

		static inline EditorMgr* instance() { return mInstance; };

		EditorTypes::Project mCurrentProject;
		EditorTypes::Scene mCurrentScene;
		std::vector<EditorTypes::Project> mAllProjects;

		void InitializeProject(EditorTypes::Project project);

		

		UIState uiState = UI_STATE_PROJECTS;

	private:
		static EditorMgr* mInstance;
	};
}