#include <RootTask.h>

#include <rio.h>
#include <gfx/rio_Projection.h>
#include <gfx/rio_Window.h>
#include <string>
#include <stdio.h>

#include <helpers/common/NodeMgr.h>
#include <helpers/common/FFLMgr.h>
#include <helpers/editor/EditorMgr.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <helpers/ui/ThemeMgr.h>

RootTask::RootTask() : ITask("FFL Testing"), mInitialized(false)
{
}

void RootTask::prepare_()
{
    // Init imgui
    initImgui();

    mInitialized = false;

    EditorMgr::instance()->SetupFrameBuffer();
    FFLMgr::instance()->InitializeFFL();
    NodeMgr::instance()->LoadFromFile("testMap.yaml");
    NodeMgr::instance()->Start();

    mInitialized = true;
}

void RootTask::calc_()
{
    if (!mInitialized)
        return;

    EditorMgr::instance()->Update();
    NodeMgr::instance()->Update();
    EditorMgr::instance()->CreateEditorUI();
}

void RootTask::exit_()
{
    if (!mInitialized)
        return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    mInitialized = false;
}

void RootTask::initImgui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ThemeMgr::createSingleton();
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("./fs/content/font/editor_main.ttf", 17);

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ThemeMgr::instance()->applyTheme(ThemeMgr::instance()->sDefaultTheme);
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(rio::Window::instance()->getNativeWindow().getGLFWwindow(), true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Setup display sizes and scales
    io.DisplaySize.x = (float)rio::Window::instance()->getWidth();  // set the current display width
    io.DisplaySize.y = (float)rio::Window::instance()->getHeight(); // set the current display height here

    rio::Window::instance()->setOnResizeCallback(&RootTask::onResizeCallback_);
}

void RootTask::resize_(s32 width, s32 height)
{
    ImGuiIO &io = ImGui::GetIO();

    // Setup display sizes and scales
    io.DisplaySize.x = (float)width;  // set the current display width
    io.DisplaySize.y = (float)height; // set the current display height here
}

void RootTask::onResizeCallback_(s32 width, s32 height)
{
    static_cast<RootTask *>(rio::sRootTask)->resize_(width, height);
}
