#include "EditorTask.h"
#include "UI/ProjectsUI.h"
#include "rio-e/EditorMgr.h"

#include "gfx/rio_Window.h"
#include "gfx/rio_Graphics.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "filedevice/rio_FileDeviceMgr.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>

EditorTask::EditorTask() : ITask("RIO(e)")
{
}

void EditorTask::prepare_()
{
	RIO_LOG("[RIO(e)] EditorTask preparing..\n");

	InitializeImGui();

	std::string persistentDataPath = std::filesystem::current_path().string() + "/persistentData";

	// If there is no persistentData folder, we'll need to create it to store all data like a list of
	// projects, and different developer preferences.
	if (!std::filesystem::exists(persistentDataPath))
	{
		std::filesystem::create_directory(persistentDataPath);
		RIO_LOG("[RIO(e)] Created persistentData.\n");

		char* projectsYAMLTemplate = (char*)"projects:";
		char* preferencesYAMLTemplate = (char*)"preferences:";

		rio::FileHandle fileHandle;
		rio::FileDeviceMgr::instance()->getNativeFileDevice()->open(&fileHandle, persistentDataPath + "/projectsList.yaml", rio::FileDevice::FILE_OPEN_FLAG_WRITE);

		fileHandle.write(reinterpret_cast<u8*>(projectsYAMLTemplate), strlen(projectsYAMLTemplate));
		fileHandle.close();

		rio::FileDeviceMgr::instance()->getNativeFileDevice()->open(&fileHandle, persistentDataPath + "/preferences.yaml", rio::FileDevice::FILE_OPEN_FLAG_WRITE);

		fileHandle.write(reinterpret_cast<u8*>(preferencesYAMLTemplate), strlen(preferencesYAMLTemplate));
		fileHandle.close();
	}

	YAML::Node projectsYAMLNode = YAML::LoadFile(persistentDataPath + "/projectsList.yaml");
	for (const auto& YAMLproject : projectsYAMLNode["projects"])
	{
		rioe::EditorTypes::Project project = rioe::EditorTypes::GetProjectData(YAMLproject.second.as<std::string>());
		rioe::EditorMgr::instance()->mAllProjects.push_back(project);
	}
}

void EditorTask::calc_()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport();

	rioe::ProjectsUI::CreateRIOeInfo();
	rioe::ProjectsUI::CreateProjectsList();

	ImGui::Render();

	rio::Graphics::setViewport(0, 0, globalIO->DisplaySize.x, globalIO->DisplaySize.y);
	rio::Window::instance()->clearColor(0, 0, 0);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorTask::exit_()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void EditorTask::InitializeImGui()
{
	RIO_LOG("[RIO(e)] ImGui initializing..\n");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	globalIO = &ImGui::GetIO();

	globalIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	globalIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	globalIO->Fonts->AddFontFromFileTTF("./fs/content/font/editor_main.ttf", 18);

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(rio::Window::instance()->getNativeWindow().getGLFWwindow(), true);
	ImGui_ImplOpenGL3_Init("#version 130");

	rio::Window::instance()->setOnResizeCallback(&EditorTask::onResizeCallback_);

	RIO_LOG("[RIO(e)] ImGui initialized!\n");
}

void EditorTask::resize_(s32 width, s32 height)
{
	globalIO->DisplaySize.x = (float)width;
	globalIO->DisplaySize.y = (float)height;
}

void EditorTask::onResizeCallback_(s32 width, s32 height)
{
	static_cast<EditorTask*>(rio::sRootTask)->resize_(width, height);
}