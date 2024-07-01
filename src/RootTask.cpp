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

#include <helpers/CameraController.h>
#include <helpers/audio/AudioNode.h>
#include <helpers/model/ModelNode.h>

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
__attribute__((aligned(rio::Drawer::cUniformBlockAlignment))) ModelNode::LightBlock RootTask::sLightBlock;

RootTask::RootTask()
    : ITask("FFL Testing"), mInitialized(false)
{
}

void RootTask::prepare_()
{
    // Init imgui
    initImgui();

    mInitialized = false;

    FFLInitDesc init_desc;
    init_desc.fontRegion = FFL_FONT_REGION_0;
    init_desc._c = false;
    init_desc._10 = true;

#if RIO_IS_CAFE
    FSInit();
#endif // RIO_IS_CAFE

    {
        std::string resPath;
        resPath.resize(256);
        // Middle
        {
            FFLGetResourcePath(resPath.data(), 256, FFL_RESOURCE_TYPE_MIDDLE, false);
            {
                rio::FileDevice::LoadArg arg;
                arg.path = resPath;
                arg.alignment = 0x2000;

                u8 *buffer = rio::FileDeviceMgr::instance()->getNativeFileDevice()->tryLoad(arg);
                if (buffer == nullptr)
                {
                    RIO_LOG("NativeFileDevice failed to load: %s\n", resPath.c_str());
                    RIO_ASSERT(false);
                    return;
                }

                mResourceDesc.pData[FFL_RESOURCE_TYPE_MIDDLE] = buffer;
                mResourceDesc.size[FFL_RESOURCE_TYPE_MIDDLE] = arg.read_size;
            }
        }
        // High
        {
            FFLGetResourcePath(resPath.data(), 256, FFL_RESOURCE_TYPE_HIGH, false);
            {
                rio::FileDevice::LoadArg arg;
                arg.path = resPath;
                arg.alignment = 0x2000;

                u8 *buffer = rio::FileDeviceMgr::instance()->getNativeFileDevice()->tryLoad(arg);
                if (buffer == nullptr)
                {
                    RIO_LOG("NativeFileDevice failed to load: %s\n", resPath.c_str());
                    RIO_ASSERT(false);
                    return;
                }

                mResourceDesc.pData[FFL_RESOURCE_TYPE_HIGH] = buffer;
                mResourceDesc.size[FFL_RESOURCE_TYPE_HIGH] = arg.read_size;
            }
        }
    }

    FFLResult result = FFLInitResEx(&init_desc, &mResourceDesc);
    if (result != FFL_RESULT_OK)
    {
        RIO_LOG("FFLInitResEx() failed with result: %d\n", (s32)result);
        RIO_ASSERT(false);
        return;
    }

    FFLiEnableSpecialMii(333326543);

    RIO_ASSERT(FFLIsAvailable());

    FFLInitResGPUStep();

    mShader.initialize();

    // Initializing random mii DB.
    {
        miiBufferSize = new u8[FFLGetMiddleDBBufferSize(100)];
        RIO_LOG("Created mii buffer size \n");
        FFLInitMiddleDB(&randomMiddleDB, FFL_MIDDLE_DB_TYPE_RANDOM_PARAM, miiBufferSize, 100);
        FFLUpdateMiddleDB(&randomMiddleDB);
        RIO_LOG("Init'd middle DB \n");
    }

    createModel_(0);

    FOV = 90.f;
    mMainBgmAudioNode = new AudioNode(mCamera, 1.0f);
    startProjectionMatrix();
    {
        // Create view uniform block instance
        mpViewUniformBlock = new rio::UniformBlock();
        // Set base data for view uniform block
        mpViewUniformBlock->setData(&sViewBlock, sizeof(ModelNode::ViewBlock));

        // Set light color
        mLightColor.set(.7f, .7f, .7f);
        // Set light position
        mLightPos.set(-8.0f, 10.0f, 20.0f);

        // Create light uniform block instance
        mpLightUniformBlock = new rio::UniformBlock();
        // Set light uniform block data and invalidate cache now as it won't be modified
        sLightBlock.light_color = mLightColor;
        sLightBlock.light_pos = mLightPos;
        mpLightUniformBlock->setDataInvalidate(&sLightBlock, sizeof(ModelNode::LightBlock));

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
    isDebuggingOpen = false;
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
        .data = &randomMiddleDB,
        .index = index};

    mpModel = new Model();
    mpModel->initialize(arg, mShader);
    mpModel->setScale({1 / 17.f, 1 / 17.f, 1 / 17.f});
}

void RootTask::calc_()
{
    if (!mInitialized)
        return;

    rio::Controller *controller = rio::ControllerMgr::instance()->getGamepad(0);
    // If there is no controller connected, try getting the "main" controller. (credits to abood's FFL-Testing)
    if (!controller || !controller->isConnected())
    {
        controller = rio::ControllerMgr::instance()->getMainGamepad();
        RIO_ASSERT(controller);
    }

    if (controller->isConnected())
    {
        controller->calc();
        useFlyCam(&mCamera, controller);
    }

    rio::Window::instance()->clearColor(0.2f, 0.3f, 0.3f, 0.0f);
    rio::Window::instance()->clearDepthStencil();

    mMainBgmAudioNode->UpdateAudio(mCamera);

    // Get view matrix
    rio::Matrix34f view_mtx;
    mCamera.getMatrix(&view_mtx);

    // Set primitive renderer camera
    rio::PrimitiveRenderer::instance()->setCamera(mCamera);

    // Calculate view-projection matrix (Projection x View)
    rio::Matrix44f view_proj_mtx;
    view_proj_mtx.setMul(mProjMtx, view_mtx);

    // Update view uniform block
    sViewBlock.view_pos = mCamera.pos();
    sViewBlock.view_proj_mtx = view_proj_mtx;
    mpViewUniformBlock->setSubDataInvalidate(&sViewBlock, 0, sizeof(ModelNode::ViewBlock));

    // Restore default GPU render state
    rio::RenderState render_state;
    render_state.apply();

    mMainModelNode->Draw(*mpViewUniformBlock, *mpLightUniformBlock);
    rio::PrimitiveRenderer::instance()->begin();
    {
        rio::PrimitiveRenderer::instance()->drawSphere8x16(
            mLightPos, 0.125f,
            rio::Color4f{mLightColor.x, mLightColor.y, mLightColor.z, 1.0f});
    }
    rio::PrimitiveRenderer::instance()->end();

    Render();
}

void RootTask::Render()
{
    // Get view matrix
    rio::BaseMtx34f view_mtx;
    mCamera.getMatrix(&view_mtx);

    mpModel->drawOpa(view_mtx, mProjMtx);
    mpModel->drawXlu(view_mtx, mProjMtx);

    // Rendering GUI
    {
        ImGuiIO &io = *p_io;

#if RIO_IS_CAFE
        ImGui_ImplWiiU_ControllerInput input;

        getCafeVPADDevice(&input);
        getCafeKPADDevice(&input);
        ImGui_ImplWiiU_ProcessInput(&input);

        ImGui_ImplGX2_NewFrame();
#else
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
#endif // RIO_IS_CAFE
        ImGui::NewFrame();
        {
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Save Mii..", "Ctrl+S"))
                    {
                        FFLStoreData store_data;
                        FFLiGetStoreData(&store_data, FFL_DATA_SOURCE_MIDDLE_DB, 52);
                        rio::FileDevice *fileDevice = rio::FileDeviceMgr::instance()->getMainFileDevice();
                        rio::FileHandle fileHandle;
                        fileDevice->open(&fileHandle, "TestMii.ffsd", rio::FileDevice::FILE_OPEN_FLAG_CREATE);
                        fileDevice->write(&fileHandle, store_data.data, sizeof(store_data.data));
                        fileDevice->close(&fileHandle);
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Camera Utility"))
                {
                    ImGui::Text("Camera X: %f", mCamera.pos().x);
                    ImGui::Text("Camera Y: %f", mCamera.pos().y);
                    ImGui::Text("Camera Z: %f", mCamera.pos().z);
                    ImGui::Text("Look At X: %f", mCamera.at().x);
                    ImGui::Text("Look At Y: %f", mCamera.at().y);
                    ImGui::Text("Look At Z: %f", mCamera.at().z);

                    // ImGui::Text("Stick X: %f", controller->getLeftStick().x);
                    // ImGui::Text("Stick Y: %f", controller->getLeftStick().y);

                    if (ImGui::SliderFloat("FOV", &FOV, .5f, 100.f))
                    {
                        // updateProjectionMatrix();
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Mii Utility"))
                {
                    ImGui::Text("Random Mii Database Size: %d", FFLGetMiddleDBStoredSize(&randomMiddleDB));
                    if (ImGui::Button("Select Random Mii"))
                    {
                        delete mpModel;
                        createModel_(rand() % FFLGetMiddleDBStoredSize(&randomMiddleDB));
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Debug"))
                {
                    isDebuggingOpen = true;

                    ImGui::ShowDebugLogWindow(&isDebuggingOpen);

                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            ImGui::End();
        }
        ImGui::Render();

#if RIO_IS_CAFE
        ImGui_ImplGX2_RenderDrawData(ImGui::GetDrawData());
#else
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif // RIO_IS_CAFE

#if RIO_IS_CAFE
        // Render keyboard overlay
        rio::Graphics::setViewport(0, 0, io.DisplaySize.x, io.DisplaySize.y, 0.0f, 1.0f);
        rio::Graphics::setScissor(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        ImGui_ImplWiiU_DrawKeyboardOverlay();
#else
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
#endif // RIO_IS_CAFE
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

    // Check if mpModel is not null before deleting it.
    delete mpModel; // FFLCharModel destruction must happen before FFLExit
    delete[] miiBufferSize;
    delete mMainBgmAudioNode;
    delete mMainModelNode;

    FFLExit();

    // Free the resources and set pointers to nullptr.
    if (mResourceDesc.pData[FFL_RESOURCE_TYPE_HIGH])
    {
        rio::MemUtil::free(mResourceDesc.pData[FFL_RESOURCE_TYPE_HIGH]);
        mResourceDesc.pData[FFL_RESOURCE_TYPE_HIGH] = nullptr;
    }
    if (mResourceDesc.pData[FFL_RESOURCE_TYPE_MIDDLE])
    {
        rio::MemUtil::free(mResourceDesc.pData[FFL_RESOURCE_TYPE_MIDDLE]);
        mResourceDesc.pData[FFL_RESOURCE_TYPE_MIDDLE] = nullptr;
    }

    mInitialized = false;
}

void RootTask::initImgui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    p_io = &io;
    io.Fonts->AddFontFromFileTTF("NotoSans-Regular.ttf", 20);
#if RIO_IS_CAFE
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
#else
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
#endif                                                // RIO_IS_CAFE
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

#if !RIO_IS_CAFE
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
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

void RootTask::updateProjectionMatrix()
{
    // Get window instance
    const rio::Window *const window = rio::Window::instance();

    // Create perspective projection instance
    rio::PerspectiveProjection proj(
        0.1f,
        100.0f,
        rio::Mathf::deg2rad(FOV),
        f32(window->getWidth()) / f32(window->getHeight()));

    // Calculate matrix
    rio::MemUtil::copy(&mProjMtx, &proj.getMatrix(), sizeof(rio::Matrix44f));
}

void RootTask::startProjectionMatrix()
{
    // Get window instance
    const rio::Window *const window = rio::Window::instance();

    // Create perspective projection instance
    rio::PerspectiveProjection proj(
        0.1f,
        100.0f,
        rio::Mathf::deg2rad(FOV),
        f32(window->getWidth()) / f32(window->getHeight()));

    // Calculate matrix
    rio::MemUtil::copy(&mProjMtx, &proj.getMatrix(), sizeof(rio::Matrix44f));

    // Set primitive renderer projection
    rio::PrimitiveRenderer::instance()->setProjection(proj);
}

#if RIO_IS_WIN

void RootTask::resize_(s32 width, s32 height)
{
    ImGuiIO &io = *p_io;

    // Setup display sizes and scales
    io.DisplaySize.x = (float)width;  // set the current display width
    io.DisplaySize.y = (float)height; // set the current display height here

    updateProjectionMatrix();
}

void RootTask::onResizeCallback_(s32 width, s32 height)
{
    static_cast<RootTask *>(rio::sRootTask)->resize_(width, height);
}

#endif // RIO_IS_WIN
