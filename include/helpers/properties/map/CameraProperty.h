#ifndef CAMERAHELPER_H
#define CAMERAHELPER_H

#include <rio.h>

#include <gfx/rio_Color.h>
#include <gfx/rio_Camera.h>
#include <gfx/rio_Camera.h>
#include <gfx/rio_Projection.h>
#include <gfx/rio_PrimitiveRenderer.h>
#include <gfx/rio_Window.h>

#include <audio/rio_AudioMgr.h>

#include <controller/rio_Controller.h>
#include <controller/rio_ControllerMgr.h>
#include <controller/win/rio_KeyboardMouseDeviceWin.h>
#include <controller/win/rio_WinControllerWin.h>

#include <misc/rio_MemUtil.h>

#include <helpers/properties/Property.h>
#include <helpers/common/Node.h>
#include <helpers/editor/EditorTypes.h>


class CameraProperty : public Property
{
public:
    enum CameraType
    {
        CAMERA_NODE_FLYCAM = 0,
        CAMERA_NODE_NONE = 1
    };

    EnumInfo CameraTypeInfo[2] = {
        {"Flycam", CAMERA_NODE_FLYCAM},
        {"Custom Controlled", CAMERA_NODE_NONE}};

    struct CameraPropertyInitArgs
    {
        CameraType pCameraType = CAMERA_NODE_FLYCAM;
        f32 FOV = 70.f;
        rio::Color4f pClearColor = {0.2f, 0.3f, 0.3f, 0.0f};
    };

    using Property::Property;

    void Load(YAML::Node node);
    YAML::Node Save() override;

    void Update() override;
    void Start() override;
    void CreatePropertiesMenu() override;

    inline rio::LookAtCamera GetCamera() { return mCamera; };
    inline rio::Matrix44f GetProjectionMatrix() { return mProjMtx; };

private:
    void UseFlyCam();

    CameraType mCameraType;
    rio::LookAtCamera mCamera;
    rio::Matrix44f mProjMtx;
    rio::Color4f mClearColor = {0.2f, 0.3f, 0.3f, 0.0f};
    f32 fov = 90.f;
};

#endif // CAMERAHELPER_H