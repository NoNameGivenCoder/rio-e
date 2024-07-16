#include <rio.h>
#include <audio/rio_AudioMgr.h>
#include <audio/rio_AudioSrc.h>
#include <filedevice/rio_FileDeviceMgr.h>
#include <gfx/rio_Camera.h>
#include <helpers/audio/AudioProperty.h>
#include <helpers/common/Node.h>
#include <helpers/common/CameraNode.h>
#include <helpers/common/NodeMgr.h>
#include <yaml-cpp/yaml.h>
#include <imgui.h>

void AudioProperty::Load(YAML::Node node)
{
    audioFile = node["audioFile"].as<std::string>();
    volume = node["volume"].as<f32>();
    audioType = (AudioProperty::AudioType)(node["audioType"].as<int>());

    RIO_LOG("[AUDIO] Loading %s..\n", audioFile.c_str());

    switch (audioType)
    {
    default:
    case AUDIO_PROPERTY_BGM:
    {
        bool loadBgmResult = rio::AudioMgr::instance()->loadBgm(audioFile.c_str(), audioFile.c_str());
        RIO_ASSERT(loadBgmResult);
        break;
    }

    case AUDIO_PROPERTY_SFX:
    {
        bool loadSfxResult = rio::AudioMgr::instance()->loadSfx(audioFile.c_str(), audioFile.c_str());
        RIO_ASSERT(loadSfxResult);
        break;
    }
    }

    RIO_LOG("[AUDIO] Loaded %s using key: %s.\n", audioFile.c_str(), audioFile.c_str());

    // rio::AudioMgr::instance()->setListener(args.camera.mCamera.pos(), args.camera.mCamera.at(), args.camera.mCamera.getUp());
}

void AudioProperty::CreatePropertiesMenu()
{
    std::string label = "Audio (" + audioFile + ")";

    if (ImGui::CollapsingHeader(label.c_str()))
    {
        std::string audioLabel = "volume_" + audioFile;
        std::string playLabel = "Play " + audioFile;

        if (ImGui::DragFloat(audioLabel.c_str(), &volume, 0.01f, 0.f, 1.f))
            SetVolume(volume);

        if (ImGui::Button(playLabel.c_str(), {ImGui::GetItemRectSize().x, 25}))
        {
            Play();
        }
    }
}

void AudioProperty::Update()
{
    // rio::AudioMgr::instance()->setListener(camera->mCamera.pos(), camera->mCamera.at(), camera->mCamera.getUp());
}

void AudioProperty::Play(const bool loop)
{
    switch (audioType)
    {
    case AUDIO_PROPERTY_BGM:
    {
        rio::AudioBgm *bgm = rio::AudioMgr::instance()->getBgm(audioFile.c_str());
        bgm->setVolume(volume);
        bgm->play(loop);
        break;
    }

    case AUDIO_PROPERTY_SFX:
    {
        rio::AudioSfx *sfx = rio::AudioMgr::instance()->getSfx(audioFile.c_str());
        sfx->setVolume(volume);
        sfx->play(loop);
        break;
    }
    }

    RIO_LOG("[AUDIO] Played %s.\n", audioFile.c_str());
}

void AudioProperty::SetVolume(const f32 volume)
{
    switch (audioType)
    {
    case AUDIO_PROPERTY_BGM:
    {
        rio::AudioBgm *bgm = rio::AudioMgr::instance()->getBgm(audioFile.c_str());
        bgm->setVolume(volume);
        break;
    }

    case AUDIO_PROPERTY_SFX:
    {
        rio::AudioSfx *sfx = rio::AudioMgr::instance()->getSfx(audioFile.c_str());
        sfx->setVolume(volume);
        break;
    }
    }
}