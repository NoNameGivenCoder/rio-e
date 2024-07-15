#include <helpers/common/NodeMgr.h>
#include <helpers/common/CameraNode.h>
#include <helpers/model/LightNode.h>
#include <helpers/ui/editor/menu/MainMenuBar.h>
#include <vector>
#include <filedevice/rio_FileDevice.h>
#include <filedevice/rio_FileDeviceMgr.h>
#include <gfx/rio_Window.h>

#include <string>
#include <iostream>
#include <stdio.h>
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

void CreateMenuBar()
{
}

void CreateContentMenu()
{
    // Get the viewport size to calculate the bottom position
    ImGuiIO &io = ImGui::GetIO();
}