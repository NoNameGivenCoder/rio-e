#ifndef CAMERAHELPER_H
#define CAMERAHELPER_H

#include <rio.h>
#include <audio/rio_AudioMgr.h>
#include <audio/rio_AudioSrc.h>
#include <gfx/rio_Camera.h>
#include <helpers/properties/Property.h>
#include <gfx/rio_Color.h>

class CameraProperty : public Property
{
public:
    enum CameraType
    {
        CAMERA_NODE_FLYCAM = 0,
        CAMERA_NODE_NONE = 1
    };

    struct CameraPropertyInitArgs
    {
        CameraType pCameraType = CAMERA_NODE_FLYCAM;
        f32 FOV = 70.f;
        rio::Color4f pClearColor = {0.2f, 0.3f, 0.3f, 0.0f};
    };

    using Property::Property;

    void Load(YAML::Node node);
    void Update() override;

    inline rio::LookAtCamera GetCamera() { return mCamera; };

private:
    void UseFlyCam();

    CameraType mCameraType;
    rio::LookAtCamera mCamera;
    rio::Matrix44f mProjMtx;
};

#endif // CAMERAHELPER_H