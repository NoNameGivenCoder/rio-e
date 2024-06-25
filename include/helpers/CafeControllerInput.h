#include <rio.h>
#include <controller/cafe/rio_CafeVPadDeviceCafe.h>
#include <controller/cafe/rio_CafeWPadDeviceCafe.h>
#include <controller/rio_ControllerMgr.h>
#include <imgui.h>
#include <imgui_impl_gx2.h>
#include <imgui_impl_wiiu.h>

void getCafeVPADDevice(ImGui_ImplWiiU_ControllerInput *input);
void getCafeKPADDevice(ImGui_ImplWiiU_ControllerInput *input);