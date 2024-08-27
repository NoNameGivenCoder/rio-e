#include <imgui.h>
#include <rio.h>

class RootTask : public rio::ITask
{
public:
    RootTask();

private:
    void prepare_() override;
    void calc_() override;
    void exit_() override;

    void initImgui();

#if RIO_IS_DESKTOP
    void resize_(s32 width, s32 height);
    static void onResizeCallback_(s32 width, s32 height);
#endif // RIO_IS_DESKTOP

private:
    bool mInitialized;
    float FOV;
    ImGuiIO *p_io;
};
