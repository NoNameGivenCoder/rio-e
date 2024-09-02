#include <string>
#include <unordered_map>
#include <filesystem>

#include "imgui.h"

#include "rio.h"
#include "gfx/rio_Window.h"
#include "filedevice/rio_FileDeviceMgr.h"

#include "windows.h"
#include "ShlObj.h"

#include "yaml-cpp/yaml.h"

#include "rio-e/EditorTypes.h"
#include "rio-e/EditorMgr.h"
#include "EditorTask.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace rioe {
	namespace ProjectsUI {
		bool WriteFile(char* inp, std::string path)
		{
			rio::FileHandle handle;
			rio::FileDevice* fileDevice = rio::FileDeviceMgr::instance()->getNativeFileDevice();
			fileDevice->open(&handle, path, rio::FileDevice::FILE_OPEN_FLAG_WRITE);
			handle.write(reinterpret_cast<u8*>(inp), strlen(inp));
			handle.close();

			return true;
		}
		rioe::EditorTypes::Project CreateNewProject(std::filesystem::path path)
		{
			// Base rio directories
			std::filesystem::create_directory(path.string() + "/build");
			std::filesystem::create_directory(path.string() + "/build/fs");
			std::filesystem::create_directory(path.string() + "/build/fs/content");
			std::filesystem::create_directory(path.string() + "/build/fs/meta");
			std::filesystem::create_directory(path.string() + "/build/fs/code");

			// Base rio content directories
			std::filesystem::create_directory(path.string() + "/build/fs/content/textures");
			std::filesystem::create_directory(path.string() + "/build/fs/content/sounds");
			std::filesystem::create_directory(path.string() + "/build/fs/content/models");
			std::filesystem::create_directory(path.string() + "/build/fs/content/shaders");

			// RIO(e) content directories
			std::filesystem::create_directory(path.string() + "/build/fs/content/map");
			std::filesystem::create_directory(path.string() + "/build/fs/content/lang");

			// Development directories
			std::filesystem::create_directory(path.string() + "/src");
			std::filesystem::create_directory(path.string() + "/include");

			YAML::Emitter projectConfig;

			projectConfig << YAML::BeginMap << YAML::Key << "project" << YAML::BeginMap;
			projectConfig << YAML::Key << "projectName" << YAML::Value << path.filename().string();
			projectConfig << YAML::Key << "editorVersion" << YAML::Value << "1.0.0";
			projectConfig << YAML::Key << "defaultScene" << YAML::Value << "Scene01.yaml";

			projectConfig << YAML::EndMap;

			WriteFile((char*)(projectConfig.c_str()), path.string() + "/projectConfig.yaml");

			YAML::Emitter helloWorldScene;

			helloWorldScene << YAML::BeginMap << YAML::Key << "nodes" << YAML::BeginMap;
			helloWorldScene << YAML::Key << "0" << YAML::BeginMap;
			helloWorldScene << YAML::Key << "name" << YAML::Value << "Camera";

			helloWorldScene << YAML::Key << "transform" << YAML::BeginMap;

			helloWorldScene << YAML::Key << "position" << YAML::BeginMap;
			helloWorldScene << YAML::Key << "x" << YAML::Value << 0;
			helloWorldScene << YAML::Key << "y" << YAML::Value << 0;
			helloWorldScene << YAML::Key << "z" << YAML::Value << 0 << YAML::EndMap;

			helloWorldScene << YAML::Key << "rotation" << YAML::BeginMap;
			helloWorldScene << YAML::Key << "x" << YAML::Value << 0;
			helloWorldScene << YAML::Key << "y" << YAML::Value << 0;
			helloWorldScene << YAML::Key << "z" << YAML::Value << 0 << YAML::EndMap;

			helloWorldScene << YAML::Key << "scale" << YAML::BeginMap;
			helloWorldScene << YAML::Key << "x" << YAML::Value << 0;
			helloWorldScene << YAML::Key << "y" << YAML::Value << 0;
			helloWorldScene << YAML::Key << "z" << YAML::Value << 0 << YAML::EndMap << YAML::EndMap;

			helloWorldScene << YAML::Key << "properties" << YAML::BeginMap;

			YAML::Node cameraProperty;

			cameraProperty["Camera"]["cameraType"] = 0;
			cameraProperty["Camera"]["cameraFOV"] = 90;
			cameraProperty["Camera"]["propertyId"] = 0;

			helloWorldScene << YAML::Key << cameraProperty.begin()->first << YAML::Value << cameraProperty.begin()->second;

			WriteFile((char*)(helloWorldScene.c_str()), path.string() + "/build/fs/content/map/Scene01.yaml");

			return rioe::EditorTypes::GetProjectData(path);
		}
		std::string CreateFolderDialog(const char* promptTitle)
		{
			BROWSEINFOA bi = { 0 };
			CHAR folDir[MAX_PATH] = { 0 };
			bi.hwndOwner = glfwGetWin32Window(rio::Window::instance()->getNativeWindow().getGLFWwindow());
			bi.pszDisplayName = folDir;
			bi.lpszTitle = promptTitle;
			bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS;

			LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);

			if (pidl != nullptr)
			{
				// Get the selected folder path
				if (SHGetPathFromIDListA(pidl, folDir))
				{
					std::string folderPath = folDir;

					CoTaskMemFree(pidl);
					return folderPath;
				}
				CoTaskMemFree(pidl);
			}

			return "";
		}
		void CreateRIOeInfo() {
			if (ImGui::Begin("RIO(e) Info"))
			{
				if (ImGui::BeginChild("info", ImVec2(0, 0), true))
				{
					ImGui::SeparatorText("RIO(e)");
					ImGui::TextWrapped("RIO(e) is a integrated editor created to make game development using the engine, RIO easier. RIO is an engine that provides compilation options for both PC and Wii U, RIO(e) expands on this, adding a full integrated development environment, with texture viewing and manipulation, model creation and viewing, audio previewing and playback, saveable layouts, customizable properties, and much more.");
					ImGui::Dummy(ImVec2(0, 5));
					ImGui::BulletText("Placeholder bullet point.");
					ImGui::BulletText("Miiverse.");
					ImGui::BulletText("Juxt.");
					ImGui::EndChild();
				}
			}
			ImGui::End();
		}
		void CreateProjectsList()
		{
			if (ImGui::Begin("Projects", nullptr, ImGuiWindowFlags_MenuBar))
			{
				if (ImGui::BeginMenuBar())
				{
					if (ImGui::BeginMenu("File"))
					{
						if (ImGui::MenuItem("New"))
						{
							RIO_LOG("[Projects] Creating new project folder..\n");

							std::filesystem::path path(CreateFolderDialog("Select a destination project folder.."));

							if (std::filesystem::exists(path.string() + "/build/fs"))
							{
								RIO_LOG("[Projects] Project already exists!\n");
							}
							else
							{
								rioe::EditorMgr::instance()->InitializeProject(CreateNewProject(path));
							}
						}

						if (ImGui::MenuItem("Open"))
						{
							RIO_LOG("[Projects] Opening new project folder..\n");
							std::filesystem::path path(CreateFolderDialog("Select a project folder.."));

							if (!path.string().empty())
							{
								auto project = rioe::EditorTypes::GetProjectData(path);

								if (!project.projectName.empty())
								{
									rioe::EditorMgr::instance()->InitializeProject(project);
								}
							}
						}

						ImGui::EndMenu();
					}

					ImGui::EndMenuBar();
				}

				if (ImGui::BeginChild("projects_list", ImVec2(0, 0), true))
				{
					int index = 0;
					for (auto& project : EditorMgr::instance()->mAllProjects)
					{
						std::string uniqueID = project.projectName + "##" + std::to_string(index++);

						if (ImGui::Selectable(uniqueID.c_str()))
							EditorMgr::instance()->InitializeProject(project);
					}
				}
				ImGui::EndChild();
			}
			ImGui::End();

			if (ImGui::BeginPopupModal("Test!", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoMove))
			{
				if (ImGui::Button("Close"))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}
	}
}