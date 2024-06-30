#ifndef AUDIOHELPER_H
#define AUDIOHELPER_H

#include <rio.h>
#include <audio/rio_AudioMgr.h>
#include <audio/rio_AudioSrc.h>
#include <gfx/rio_Camera.h>

class AudioNode
{
public:
    AudioNode(const rio::LookAtCamera &camera, f32 masterVol);
    ~AudioNode();
    void PlaySound(const char *path, const char *key, const f32 vol = 1.f, const bool loop = false, const rio::Vector3f pos = {0, 0, 0});
    void PlayBgm(const char *path, const char *key, const f32 vol = 1.f, const bool loop = false);
    void UpdateAudio(const rio::LookAtCamera &camera);
};

#endif // AUDIOHELPER_H