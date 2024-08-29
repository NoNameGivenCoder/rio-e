#include <string>

#include "imgui.h"

#include "rio.h"

namespace EditorUI {
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
					}
						

					if (ImGui::MenuItem("Open"))
					{
						RIO_LOG("[Projects] Opening new project folder..\n");
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			if (ImGui::BeginChild("projects_list", ImVec2(0, 0), true))
			{
				for (int i = 0; i < 100; i++)
				{
					ImGui::Selectable("Project Name");
				}

				ImGui::EndChild();
			}

			ImGui::End();
		}
	}
}