#include <Shader.h>

#include <gfx/rio_Camera.h>
#include <controller/rio_Controller.h>
#include <task/rio_Task.h>
#include <nn/ffl/FFLMiddleDB.h>
#include <nn/ffl/FFLMiddleDBType.h>
#include <imgui.h>

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
    void updateProjectionMatrix();
    void initImgui();
    void Render();

#if RIO_IS_WIN
    void
    resize_(s32 width, s32 height);
    static void onResizeCallback_(s32 width, s32 height);
#endif // RIO_IS_WIN

private:
    bool mInitialized;
    FFLResourceDesc mResourceDesc;
    Shader mShader;
    rio::BaseMtx44f mProjMtx;
    rio::LookAtCamera mCamera;
    rio::Vector3f CAM_POS;
    rio::Vector3f LOOK_POS;
    Model *mpModel;
    FFLMiddleDB randomMiddleDB;
    void *miiBufferSize;
    ImGuiIO *p_io;
    rio::Controller *controller;

    bool isOpen;
};
