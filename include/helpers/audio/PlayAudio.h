#ifndef AUDIOHELPER_H
#define AUDIOHELPER_H

#include <rio.h>
#include <audio/rio_AudioMgr.h>
#include <audio/rio_AudioSrc.h>
#include <gfx/rio_Camera.h>

class AudioHelper
{
public:
    AudioHelper(const rio::LookAtCamera &camera, f32 masterVol);
    ~AudioHelper();
    void PlaySound(const char *path, const char *key, const f32 vol = 1.f, bool loop = false, const rio::Vector3f pos = {0, 0, 0});
    void UpdateAudio(const rio::LookAtCamera &camera);
};

#endif // AUDIOHELPER_H