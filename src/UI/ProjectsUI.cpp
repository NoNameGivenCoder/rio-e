#include <string>
#include <unordered_map>

#include "imgui.h"

#include "rio.h"
#include "gfx/rio_Window.h"

#include "windows.h"
#include "ShlObj.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace EditorUI {
	std::string CreateFolderDialog()
	{
		BROWSEINFOA bi = { 0 };
		CHAR folDir[MAX_PATH] = { 0 };
		bi.hwndOwner = glfwGetWin32Window(rio::Window::instance()->getNativeWindow().getGLFWwindow());
		bi.pszDisplayName = folDir;
		bi.lpszTitle = "Select a folder";
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
				ImGui::TextWrapped("RIO(e) is a integrated editor created to make game development using the engine, rio easier. RIO is an engine that provides compilation options for both PC and Wii U, RIO(e) expands on this, adding a full integrated development environment, with texture viewing and manipulation, model creation and viewing, audio previewing and playback, saveable layouts, customizable properties, and much more.");
				ImGui::Dummy(ImVec2(0, 5));
				ImGui::BulletText("Placeholder bullet point.");
				ImGui::BulletText("Miiverse.");
				ImGui::BulletText("Juxt.");
				ImGui::EndChild();
			}

			ImGui::End();
		}
	}
	void CreateProjectsList(std::unordered_map<std::string, std::string>& projectsList)
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
					}
						

					if (ImGui::MenuItem("Open"))
					{
						RIO_LOG("[Projects] Opening new project folder..\n");
						std::string result = CreateFolderDialog();

						projectsList.emplace(result, result);

						RIO_LOG("%s\n", result.c_str());
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			if (ImGui::BeginChild("projects_list", ImVec2(0, 0), true))
			{
				int index = 0;
				for (const auto& project : projectsList)
				{
					std::string uniqueID = project.first + "##" + std::to_string(index++);

					ImGui::Selectable(uniqueID.c_str());
				}

				ImGui::EndChild();
			}

			ImGui::End();
		}
	}
}