#include <gfx/rio_Camera.h>
#include <controller/rio_Controller.h>
#include <task/rio_Task.h>
#include <nn/ffl/FFLMiddleDB.h>
#include <nn/ffl/FFLMiddleDBType.h>
#include <imgui.h>

#include <filedevice/rio_FileDeviceMgr.h>

class Model;

class RootTask : public rio::ITask
{
public:
    RootTask();

private:
    void prepare_() override;
    void calc_() override;
    void exit_() override;

    void createModel_(u16 index);
    void initImgui();
    void Render();

#if RIO_IS_WIN
    void resize_(s32 width, s32 height);
    static void onResizeCallback_(s32 width, s32 height);
#endif // RIO_IS_WIN

private:
    bool mInitialized;
    float FOV;
    ImGuiIO *p_io;

    bool isDebuggingOpen;
};
