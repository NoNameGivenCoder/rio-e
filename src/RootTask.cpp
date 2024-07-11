#include <Model.h>
#include <RootTask.h>

#include <filedevice/rio_FileDeviceMgr.h>
#include <filedevice/rio_FileDevice.h>
#include <rio.h>
#include <controller/rio_Controller.h>
#include <controller/rio_ControllerMgr.h>
#include <controller/win/rio_WinControllerWin.h>
#include <gfx/rio_Projection.h>
#include <gfx/rio_Window.h>
#include <string>
#include <stdio.h>
#include <gfx/mdl/res/rio_ModelCacher.h>
#include <gfx/rio_PrimitiveRenderer.h>
#include <gpu/rio_Drawer.h>
#include <gpu/rio_RenderState.h>

#include <helpers/audio/AudioNode.h>
#include <helpers/model/ModelNode.h>
#include <helpers/model/LightNode.h>

#include <helpers/ui/editor/menu/MainMenuBar.h>
#include <helpers/common/NodeMgr.h>
#include <helpers/common/FFLMgr.h>
#include <filesystem>

#if RIO_IS_CAFE
#include <controller/rio_ControllerMgr.h>
#include <controller/cafe/rio_CafeVPadDeviceCafe.h>
#include <controller/cafe/rio_CafeWPadDeviceCafe.h>
#include <helpers/CafeControllerInput.h>
#include <imgui_impl_gx2.h>
#include <imgui_impl_wiiu.h>
#include <coreinit/debug.h>
#else
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif // RIO_IS_CAFE
#include <format>
#include <helpers/ui/ThemeMgr.h>

__attribute__((aligned(rio::Drawer::cUniformBlockAlignment))) ModelNode::ViewBlock RootTask::sViewBlock;

RootTask::RootTask() : ITask("FFL Testing"), mInitialized(false)
{
}

void RootTask::prepare_()
{
    // Init imgui
    initImgui();

    mInitialized = false;

    FFLMgr::instance()->InitializeFFL();
    FFLMgr::instance()->CreateRandomMiddleDB(100);

    mShader.initialize();

    createModel_(0);

    FOV = 90.f;

    mCamera = new CameraNode("mainCameraNode", {0, 0, 0}, {0, 0, 0}, {1, 1, 1});
    mCamera->Init({CameraNode::CAMERA_NODE_FLYCAM, 90.f});
    mMainBgmAudioNode = new AudioNode("mainBgmAudioNode", {0, 0, 0}, {0, 0, 0}, {1, 1, 1});
    {
        // Set light uniform block data and invalidate cache now as it won't be modified
        mLightNode = new LightNode("mainLightNode", {0, 0, 0}, {0, 0, 0}, {1, 1, 1});
        mLightNode->Init({{1.f, 1.f, 1.f, 1.f}, LightNode::LIGHT_NODE_VISIBLE, LightNode::LIGHT_NODE_SPHERE, 0.5f});

        // Load coin model
        rio::mdl::res::Model *mario_res_mdl = rio::mdl::res::ModelCacher::instance()->loadModel("MiiBody", "miiMarioBody");

        f32 angle = rio::Mathf::deg2rad(20) * 1;

        // Model (local-to-world) matrix
        rio::Matrix34f model_mtx;
        model_mtx.makeST(
            {0.7f, 0.7f, 0.7f},
            {0, -7.8f, 0});

        mMainModelNode = new ModelNode(mario_res_mdl, "cViewBlock", "cLightBlock", "cModelBlock");

        // Set model's local-to-world matrix
        mMainModelNode->setModelWorldMtx(model_mtx);
    }

    mMainBgmAudioNode->PlayBgm("MAIN_THEME_NIGHT.mp3", "mainThemeNight", 0.2f, true);
    mInitialized = true;
}

void RootTask::createModel_(u16 index)
{
    Model::InitArgMiddleDB arg = {
        .desc = {
            .resolution = FFLResolution(2048),
            .expressionFlag = 1,
            .modelFlag = 1 << 0 | 1 << 1 | 1 << 2,
            .resourceType = FFL_RESOURCE_TYPE_HIGH,
        },
        .data = &FFLMgr::instance()->mMiddleDB,
        .index = index};

    mpModel = new Model();
    mpModel->initialize(arg, mShader);
    mpModel->setScale({1 / 17.f, 1 / 17.f, 1 / 17.f});
}

void RootTask::calc_()
{
    if (!mInitialized)
        return;

    mCamera->Update();
    mMainBgmAudioNode->UpdateAudio(mCamera);
    // Get view matrix
    rio::BaseMtx34f view_mtx;
    mCamera->mCamera.getMatrix(&view_mtx);

    mpModel->drawOpa(view_mtx, mCamera->mProjMtx);
    mpModel->drawXlu(view_mtx, mCamera->mProjMtx);
    mMainModelNode->Draw();
    mLightNode->Draw();

    Render();
}

void RootTask::Render()
{
    // Rendering GUI
    {
        ImGuiIO &io = *p_io;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        {
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    ImGui::MenuItem("Create New Scene");
                    ImGui::MenuItem("Save Scene", "Ctrl+S");
                    ImGui::MenuItem("Open Scene", "Ctrl+O");
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Edit"))
                {
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            ImVec2 layoutPos = ImVec2(0, 23.f);
            ImVec2 layoutSize = ImVec2(300.0f, io.DisplaySize.y - layoutPos.y);

            ImGui::SetNextWindowPos(layoutPos);
            ImGui::SetNextWindowSize(layoutSize);

            if (ImGui::Begin("Layout", __null, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
            {
                if (ImGui::BeginChild("nodes", ImGui::GetContentRegionAvail(), ImGuiChildFlags_AutoResizeY))
                {
                    for (const auto &node : NodeMgr::instance()->mNodes)
                    {
                        if (!node || !node->nodeKey)
                            continue;

                        bool isNodeSelected = (NodeMgr::instance()->GetSelectedNode() == node.get());

                        if (isNodeSelected)
                            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);

                        if (ImGui::Button(node.get()->nodeKey, ImVec2(ImGui::GetItemRectSize().x, 25)))
                            NodeMgr::instance()->SetSelectedNode(node.get());

                        if (isNodeSelected)
                            ImGui::PopStyleColor();
                    }

                    ImGui::EndChild();
                }

                ImGui::End();
            }

            ImVec2 propertiesSize = ImVec2(300.0f, io.DisplaySize.y - layoutPos.y);
            ImVec2 propertiesPosition = ImVec2(io.DisplaySize.x - propertiesSize.x, layoutPos.y);

            ImGui::SetNextWindowPos(propertiesPosition);
            ImGui::SetNextWindowSize(propertiesSize);

            if (ImGui::Begin("Properties", __null, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
            {
                if (ImGui::BeginChild("nodes", ImGui::GetContentRegionAvail(), ImGuiChildFlags_AutoResizeY))
                {
                    if (NodeMgr::instance()->GetSelectedNode())
                    {
                        ImGui::Text("Selected node: %s", NodeMgr::instance()->GetSelectedNode()->nodeKey);
                    }
                }

                ImGui::End();
            }

            ImVec2 assetsSize = ImVec2(io.DisplaySize.x - layoutSize.x - propertiesSize.x, 340.0f); // Adjust the height as needed
            ImVec2 assetsPos = ImVec2(layoutSize.x, io.DisplaySize.y - assetsSize.y - ImGui::GetStyle().WindowPadding.y - ImGui::GetStyle().FramePadding.y - 10);

            ImGui::SetNextWindowPos(assetsPos);
            ImGui::SetNextWindowSize(assetsSize);

            if (ImGui::Begin("Assets", __null, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
            {
                if (ImGui::BeginChild("folders", {200, ImGui::GetContentRegionAvail().y}, ImGuiChildFlags_AutoResizeY))
                {
                    for (const auto &entry : std::filesystem::directory_iterator(rio::FileDeviceMgr::instance()->getMainFileDevice()->getContentNativePath()))
                    {
                        if (!entry.is_directory())
                            continue;

                        std::string pathName = entry.path().filename().string();
                        pathName.append("/");

                        if (ImGui::Button(pathName.c_str(), ImVec2(ImGui::GetItemRectSize().x, 25)))
                        {
                            // Button clicked logic
                        }
                    }

                    ImGui::EndChild();
                }

                ImGui::SameLine();

                if (ImGui::BeginChild("contents", ImGui::GetContentRegionAvail()))
                {
                    for (const auto &entry : std::filesystem::directory_iterator(rio::FileDeviceMgr::instance()->getMainFileDevice()->getContentNativePath()))
                    {
                        std::string pathName = entry.path().filename().string();

                        if (entry.is_directory())
                            pathName.append("/");

                        ImGui::Button(pathName.c_str(), ImVec2(ImGui::GetItemRectSize().x, 25));
                    }

                    ImGui::EndChild();
                }
            }

            ImGui::End();
        }
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }
}

void RootTask::exit_()
{
    if (!mInitialized)
        return;

#if RIO_IS_CAFE
    ImGui_ImplGX2_Shutdown();
    ImGui_ImplWiiU_Shutdown();
    ImGui_ImplGX2_DestroyFontsTexture();
#else
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
#endif // RIO_IS_CAFE
    ImGui::DestroyContext();
    ThemeMgr::destroySingleton();

    // Check if mpModel is not null before deleting it.
    delete mpModel; // FFLCharModel destruction must happen before FFLExit
    // delete[] miiBufferSize;

    mInitialized = false;
}

void RootTask::initImgui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ThemeMgr::createSingleton();
    ImGuiIO &io = ImGui::GetIO();
    p_io = &io;
    io.Fonts->AddFontFromFileTTF("./fs/content/font/editor_main.ttf", 18);
#if RIO_IS_CAFE
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
#else
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
#endif                                                // RIO_IS_CAFE
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

#if !RIO_IS_CAFE
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ThemeMgr::instance()->applyTheme(ThemeMgr::instance()->sDefaultTheme);
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
#endif

#if RIO_IS_CAFE
    // Scale everything by 1.5 for the Wii U
    ImGui::GetStyle().ScaleAllSizes(1.5f);
    io.FontGlobalScale = 1.5f;
#endif // RIO_IS_CAFE

    // Setup platform and renderer backends
#if RIO_IS_CAFE
    ImGui_ImplWiiU_Init();
    ImGui_ImplGX2_Init();
#else
    ImGui_ImplGlfw_InitForOpenGL(rio::Window::instance()->getNativeWindow().getGLFWwindow(), true);
    ImGui_ImplOpenGL3_Init("#version 130");
#endif // RIO_IS_CAFE

    // Our state
    // show_demo_window = true;
    // show_another_window = false;
    // clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Setup display sizes and scales
    io.DisplaySize.x = (float)rio::Window::instance()->getWidth();  // set the current display width
    io.DisplaySize.y = (float)rio::Window::instance()->getHeight(); // set the current display height here

#if RIO_IS_WIN
    rio::Window::instance()->setOnResizeCallback(&RootTask::onResizeCallback_);
#endif // RIO_IS_WIN
}

#if RIO_IS_WIN

void RootTask::resize_(s32 width, s32 height)
{
    ImGuiIO &io = *p_io;

    // Setup display sizes and scales
    io.DisplaySize.x = (float)width;  // set the current display width
    io.DisplaySize.y = (float)height; // set the current display height here
}

void RootTask::onResizeCallback_(s32 width, s32 height)
{
    static_cast<RootTask *>(rio::sRootTask)->resize_(width, height);
}

#endif // RIO_IS_WIN
