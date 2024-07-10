#ifndef AUDIOHELPER_H
#define AUDIOHELPER_H

#include <rio.h>
#include <audio/rio_AudioMgr.h>
#include <audio/rio_AudioSrc.h>
#include <gfx/rio_Camera.h>
#include <helpers/common/Node.h>
#include <helpers/common/CameraNode.h>

class AudioNode : public Node
{
public:
    struct AudioNodeInitArgs
    {
        const CameraNode camera;
        f32 masterVol = 1.f;
        f32 maxListenerDistance = 5.f;
    };

    using Node::Node;

    void Init(AudioNodeInitArgs args);
    void PlaySound(const char *path, const char *key, const f32 vol = 1.f, const bool loop = false);
    void PlayBgm(const char *path, const char *key, const f32 vol = 1.f, const bool loop = false);
    void UpdateAudio(const CameraNode *camera);
};

#endif // AUDIOHELPER_H