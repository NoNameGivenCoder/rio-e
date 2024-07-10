#include <rio.h>
#include <audio/rio_AudioMgr.h>
#include <audio/rio_AudioSrc.h>
#include <filedevice/rio_FileDeviceMgr.h>
#include <gfx/rio_Camera.h>
#include <helpers/audio/AudioNode.h>
#include <helpers/common/Node.h>
#include <helpers/common/CameraNode.h>

void AudioNode::Init(AudioNodeInitArgs args)
{
    rio::AudioMgr::instance()->setListener(args.camera.mCamera.pos(), args.camera.mCamera.at(), args.camera.mCamera.getUp());
    rio::AudioMgr::instance()->setMasterVolume(args.masterVol);
    rio::AudioMgr::instance()->setListenerMaxDistance(args.maxListenerDistance);
}

void AudioNode::UpdateAudio(const CameraNode *camera)
{
    rio::AudioMgr::instance()->setListener(camera->mCamera.pos(), camera->mCamera.at(), camera->mCamera.getUp());
}

void AudioNode::PlaySound(const char *path, const char *key, const f32 vol, const bool loop)
{
    rio::AudioSfx *sfx = rio::AudioMgr::instance()->getSfx(key);

    if (!sfx)
    {
        RIO_LOG("[AUDIONODE] Audio SFX %s has not been loaded in prior, loading now..\n", key);
        bool loadResult = rio::AudioMgr::instance()->loadSfx(path, key);
        RIO_ASSERT(loadResult);

        sfx = rio::AudioMgr::instance()->getSfx(key);
        RIO_LOG("[AUDIONODE] Audio SFX %s loaded!\n", key);
    }

    sfx->setVolume(vol);
    RIO_ASSERT(sfx);

    sfx->play(Node::GetPosition(), loop);
    RIO_LOG("[AUDIONODE] Played SFX: %s \n", key);
}

void AudioNode::PlayBgm(const char *path, const char *key, const f32 vol, const bool loop)
{
    rio::AudioBgm *bgm = rio::AudioMgr::instance()->getBgm(key);

    if (!bgm)
    {
        RIO_LOG("[AUDIONODE] Audio BGM %s has not been loaded in prior, loading now..\n", key);
        bool loadResult = rio::AudioMgr::instance()->loadBgm(path, key);
        RIO_ASSERT(loadResult);

        bgm = rio::AudioMgr::instance()->getBgm(key);
        RIO_LOG("[AUDIONODE] Audio BGM %s loaded!\n", key);
    }

    bgm->setVolume(vol);
    RIO_ASSERT(bgm);

    bgm->play(loop);
    RIO_LOG("[AUDIONODE] Played BGM: %s \n", key);
}