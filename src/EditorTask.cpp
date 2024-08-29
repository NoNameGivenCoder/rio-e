#include "EditorTask.h"
#include "UI/ProjectsUI.h"

#include "gfx/rio_Window.h"
#include "gfx/rio_Graphics.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

EditorTask::EditorTask() : ITask("RIO(e)")
{
}

void EditorTask::prepare_()
{
	RIO_LOG("[RIO(e)] EditorTask preparing..\n");

	InitializeImGui();
}

void EditorTask::calc_()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport();

	EditorUI::CreateRIOeInfo();
	EditorUI::CreateProjectsList();

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