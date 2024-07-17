#include <rio.h>
#include <audio/rio_AudioMgr.h>
#include <audio/rio_AudioSrc.h>
#include <filedevice/rio_FileDeviceMgr.h>
#include <gfx/rio_Camera.h>
#include <helpers/properties/audio/AudioProperty.h>
#include <helpers/common/Node.h>
#include <helpers/common/NodeMgr.h>
#include <yaml-cpp/yaml.h>
#include <imgui.h>

void AudioProperty::Load(YAML::Node node)
{
    if (!node["audioFile"])
    {
        RIO_LOG("[AUDIO] Error: Missing 'audioFile' in YAML node.\n");
        return;
    }
    audioFile = node["audioFile"].as<std::string>();

    if (!node["audioType"])
    {
        RIO_LOG("[AUDIO] Error: Missing 'audioType' in YAML node.\n");
        return;
    }
    audioType = static_cast<AudioProperty::AudioType>(node["audioType"].as<int>());

    if (!node["volume"])
    {
        RIO_LOG("[AUDIO] Error: Missing 'volume' in YAML node.\n");
        return;
    }
    volume = node["volume"].as<f32>();

    if (!node["propertyId"])
    {
        RIO_LOG("[AUDIO] Error: Missing 'propertyId' in YAML node.\n");
        return;
    }
    Property::SetPropertyID(node["propertyId"].as<int>());
    Property::SetLoggingString("AUDIO");

    return LoadAudio();
}

void AudioProperty::LoadAudio()
{
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
}

void AudioProperty::CreatePropertiesMenu()
{
    int propertyId = Property::GetPropertyID();

    std::string label = "Audio (" + std::to_string(propertyId) + ")";

    if (ImGui::CollapsingHeader(label.c_str()))
    {
        std::string audioLabel = "volume_" + propertyId;
        std::string playLabel = "Play " + propertyId;
        std::string stopLabel = "Stop " + propertyId;

        if (ImGui::DragFloat(audioLabel.c_str(), &volume, 0.01f, 0.f, 1.f))
            SetVolume(volume);

        if (ImGui::Button(playLabel.c_str(), {ImGui::GetItemRectSize().x, 22}))
            Play();

        if (ImGui::Button(stopLabel.c_str(), {ImGui::GetItemRectSize().x, 22}))
            Stop();
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

void AudioProperty::Stop()
{
    switch (audioType)
    {
    case AUDIO_PROPERTY_BGM:
    {
        rio::AudioBgm *bgm = rio::AudioMgr::instance()->getBgm(audioFile.c_str());
        bgm->stop();
        break;
    }

    case AUDIO_PROPERTY_SFX:
    {
        rio::AudioSfx *sfx = rio::AudioMgr::instance()->getSfx(audioFile.c_str());
        sfx->stop(0);
        break;
    }
    }

    RIO_LOG("[AUDIO] Stopped %s.\n", audioFile.c_str());
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