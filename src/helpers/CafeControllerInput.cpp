#include <rio.h>
#include <controller/cafe/rio_CafeVPadDeviceCafe.h>
#include <controller/cafe/rio_CafeWPadDeviceCafe.h>
#include <controller/rio_ControllerMgr.h>
#include <imgui.h>
#include <imgui_impl_gx2.h>
#include <imgui_impl_wiiu.h>

void getCafeVPADDevice(ImGui_ImplWiiU_ControllerInput *input)
{
    rio::CafeVPadDevice *device = static_cast<rio::CafeVPadDevice *>(rio::ControllerMgr::instance()->getControlDevice(rio::ControllerDefine::DEVICE_CAFE_V_PAD));
    if (device)
    {
        {
            const rio::CafeVPadDevice::VPadInfo &v_pad_info = device->getVPadInfo();
            if (v_pad_info.last_read_error == VPAD_READ_SUCCESS)
                input->vpad = const_cast<VPADStatus *>(&(v_pad_info.status[0]));
        }
    }
}
void getCafeKPADDevice(ImGui_ImplWiiU_ControllerInput *input)
{
    rio::CafeWPadDevice *device = static_cast<rio::CafeWPadDevice *>(rio::ControllerMgr::instance()->getControlDevice(rio::ControllerDefine::DEVICE_CAFE_W_PAD));
    if (device)
    {
        for (int i = 0; i < 4; i++)
        {
            const rio::CafeWPadDevice::KPadInfo &k_pad_info = device->getKPadInfo(i);
            if (k_pad_info.last_read_error == KPAD_ERROR_OK)
                input->kpad[i] = const_cast<KPADStatus *>(&(k_pad_info.status[0]));
        }
    }
}