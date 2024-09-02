#include "rio.h"

#include "imgui.h"

#include <unordered_map>
#include <string>

#include "rio-e/EditorTypes.h"

class EditorTask : public rio::ITask
{
public:
	EditorTask();

private:
	void prepare_() override;
	void calc_() override;
	void exit_() override;

	void InitializeImGui();

#if RIO_IS_WIN
	void resize_(s32 width, s32 height);
	static void onResizeCallback_(s32 width, s32 height);
#endif // RIO_IS_WIN

public:
	std::vector<rioe::EditorTypes::Project> mProjects;
	rioe::EditorTypes::Project* mSelectedProject;

private:
	bool mInitialized = false;
	ImGuiIO *globalIO;
};