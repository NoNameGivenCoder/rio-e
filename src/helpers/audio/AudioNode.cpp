#include <rio.h>
#include <audio/rio_AudioMgr.h>
#include <audio/rio_AudioSrc.h>
#include <filedevice/rio_FileDeviceMgr.h>
#include <gfx/rio_Camera.h>
#include <helpers/audio/AudioNode.h>

AudioNode::AudioNode(const rio::LookAtCamera &camera, f32 masterVol)
{
    rio::AudioMgr::instance()->setListener(camera.pos(), camera.at(), camera.getUp());
    rio::AudioMgr::instance()->setMasterVolume(masterVol);
    rio::AudioMgr::instance()->setListenerMaxDistance(5.f);
}

AudioNode::~AudioNode()
{
}

void AudioNode::UpdateAudio(const rio::LookAtCamera &camera)
{
    rio::AudioMgr::instance()->setListener(camera.pos(), camera.at(), camera.getUp());
}

void AudioNode::PlaySound(const char *path, const char *key, const f32 vol, const bool loop, const rio::Vector3f pos)
{
    rio::AudioSfx *sfx = rio::AudioMgr::instance()->getSfx(key);

    if (!sfx)
    {
        RIO_LOG("Audio SFX %s has not been loaded in prior, loading now..\n", key);
        bool loadResult = rio::AudioMgr::instance()->loadSfx(path, key);
        RIO_ASSERT(loadResult);

        sfx = rio::AudioMgr::instance()->getSfx(key);
        RIO_LOG("Audio SFX %s loaded!\n", key);
    }

    sfx->setVolume(vol);
    RIO_ASSERT(sfx);

    sfx->play(pos, loop);
    RIO_LOG("Played SFX: %s \n", key);
}

void AudioNode::PlayBgm(const char *path, const char *key, const f32 vol, const bool loop)
{
    rio::AudioBgm *bgm = rio::AudioMgr::instance()->getBgm(key);

    if (!bgm)
    {
        RIO_LOG("Audio BGM %s has not been loaded in prior, loading now..\n", key);
        bool loadResult = rio::AudioMgr::instance()->loadBgm(path, key);
        RIO_ASSERT(loadResult);

        bgm = rio::AudioMgr::instance()->getBgm(key);
        RIO_LOG("Audio BGM %s loaded!\n", key);
    }

    bgm->setVolume(vol);
    RIO_ASSERT(bgm);

    bgm->play(loop);
    RIO_LOG("Played BGM: %s \n", key);
}