#include "rio.h"

#include "imgui.h"

#include <unordered_map>
#include <string>

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

private:
	bool mInitialized = false;
	ImGuiIO *globalIO;

	std::unordered_map<std::string, std::string> mProjectFolders;
};