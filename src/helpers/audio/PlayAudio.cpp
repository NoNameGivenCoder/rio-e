#include <rio.h>
#include <audio/rio_AudioMgr.h>
#include <audio/rio_AudioSrc.h>
#include <filedevice/rio_FileDeviceMgr.h>
#include <gfx/rio_Camera.h>
#include <helpers/audio/PlayAudio.h>

AudioHelper::AudioHelper(const rio::LookAtCamera &camera, f32 masterVol)
{
    rio::AudioMgr::instance()->setListener(camera.pos(), camera.at(), camera.getUp());
    rio::AudioMgr::instance()->setMasterVolume(masterVol);
    rio::AudioMgr::instance()->setSfxVolume(masterVol);
    rio::AudioMgr::instance()->setMusicVolume(masterVol);
    rio::AudioMgr::instance()->setListenerMaxDistance(5.f);

    RIO_LOG("Initialized RIO audio. \n");
}

AudioHelper::~AudioHelper()
{
}

void AudioHelper::UpdateAudio(const rio::LookAtCamera &camera)
{
    rio::AudioMgr::instance()->setListener(camera.pos(), camera.at(), camera.getUp());
    rio::AudioMgr::instance()->setListenerMaxDistance(5.f);
}

void AudioHelper::PlaySound(const char *path, const char *key, const f32 vol, bool loop, const rio::Vector3f pos)
{
    rio::AudioSfx *sfx = rio::AudioMgr::instance()->getSfx(key);

    if (!sfx)
    {
        bool loadResult = rio::AudioMgr::instance()->loadSfx(path, key);
        RIO_ASSERT(loadResult);

        sfx = rio::AudioMgr::instance()->getSfx(key);
    }

    sfx->setVolume(vol);

    RIO_ASSERT(sfx);

    sfx->play(pos, loop);
    RIO_LOG("Played SFX: %s \n", key);
}