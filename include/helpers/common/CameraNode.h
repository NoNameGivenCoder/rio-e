#ifndef CAMERAHELPER_H
#define CAMERAHELPER_H

#include <rio.h>
#include <audio/rio_AudioMgr.h>
#include <audio/rio_AudioSrc.h>
#include <gfx/rio_Camera.h>
#include <helpers/common/Node.h>
#include <gfx/rio_Color.h>

class CameraNode : public Node
{
public:
    enum CameraType
    {
        CAMERA_NODE_FLYCAM = 0
    };

    struct CameraNodeInitArgs
    {
        CameraType pCameraType = CAMERA_NODE_FLYCAM;
        f32 FOV = 70.f;
        rio::Color4f pClearColor = {0.2f, 0.3f, 0.3f, 0.0f};
    };

    using Node::Node;

    void Init(CameraNodeInitArgs args);
    void Update();

    rio::LookAtCamera mCamera;
    rio::Matrix44f mProjMtx;

private:
    CameraType mCameraType;
};

#endif // CAMERAHELPER_H