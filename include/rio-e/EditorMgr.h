#pragma once

#include "rio-e/EditorTypes.h"

namespace rioe
{
	class EditorMgr
	{
	public:
		static bool createSingleton();
		static bool destorySingleton();

		static inline EditorMgr* instance() { return mInstance; };

		EditorTypes::Project mCurrentProject;
		EditorTypes::Scene mCurrentScene;
		std::vector<EditorTypes::Project> mAllProjects;

		void InitializeProject(EditorTypes::Project project);

	private:
		static EditorMgr* mInstance;
	};
}