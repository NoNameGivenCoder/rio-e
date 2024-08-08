#include <rio.h>
#include <audio/rio_AudioMgr.h>
#include <audio/rio_AudioSrc.h>
#include <filedevice/rio_FileDeviceMgr.h>
#include <gfx/rio_Camera.h>
#include <helpers/properties/audio/AudioProperty.h>
#include <helpers/common/Node.h>
#include <helpers/common/NodeMgr.h>
#include <helpers/properties/map/CameraProperty.h>
#include <yaml-cpp/yaml.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

YAML::Node AudioProperty::Save()
{
    YAML::Node node;

    node["Audio"]["audioFile"] = audioFile->c_str();
    node["Audio"]["audioKey"] = audioKey->c_str();
    node["Audio"]["audioType"] = (int)(audioType);
    node["Audio"]["loop"] = (int)(loop);
    node["Audio"]["volume"] = (float)(volume);
    node["Audio"]["propertyId"] = (int)(Property::GetPropertyID());

    return node;
}

void AudioProperty::Load(YAML::Node node)
{
    audioFile = std::make_shared<std::string>(node["audioFile"].as<std::string>());
    audioKey = std::make_shared<std::string>(node["audioKey"].as<std::string>());
    audioType = static_cast<AudioProperty::AudioType>(node["audioType"].as<int>());
    volume = node["volume"].as<f32>();
    loop = node["loop"].as<int>();

    Property::SetPropertyID(node["propertyId"].as<int>());
    Property::SetLoggingString("AUDIO");

    rio::AudioMgr::instance()->setListenerMaxDistance(5.f);
}

void AudioProperty::Start()
{
    LoadAudio();
    mInitialized = true;
}

void AudioProperty::LoadAudio()
{
    int propertyId = Property::GetPropertyID();

    RIO_LOG("[AUDIO] Loading %s..\n", audioFile->c_str());

    switch (audioType)
    {
    default:
    case AUDIO_PROPERTY_BGM:
    {
        audioLoaded = rio::AudioMgr::instance()->loadBgm(audioFile->c_str(), audioKey->c_str());
        RIO_ASSERT(audioLoaded);
        break;
    }

    case AUDIO_PROPERTY_SFX:
    {
        audioLoaded = rio::AudioMgr::instance()->loadSfx(audioFile->c_str(), audioKey->c_str());
        RIO_ASSERT(audioLoaded);
        break;
    }
    }

    if (audioLoaded)
        RIO_LOG("[AUDIO] Loaded successfully.\n");

    RIO_LOG("[AUDIO] Loaded %s using key: %s.\n", audioFile->c_str(), audioKey->c_str());
}

void AudioProperty::CreatePropertiesMenu()
{
    int propertyId = Property::GetPropertyID();
    ImGui::PushID(propertyId);

    if (ImGui::CollapsingHeader("Audio"))
    {
        ImGui::PopID();

        std::string volumeID = "volume_" + propertyId;
        std::string audioFileID = "audioFile_" + propertyId;

        ImGui::Text("Audio File");

        ImGui::PushID(volumeID.c_str());

        if (ImGui::InputText("", audioFile.get()))
            LoadAudio();

        ImGui::PopID();

        ImGui::Text("Audio Volume");
        ImGui::PushID(audioFileID.c_str());

        if (ImGui::DragFloat("", &volume, 0.01f, 0.f, 1.f))
            SetVolume(volume);

        ImGui::PopID();
    }
}

void AudioProperty::Update()
{
}

void AudioProperty::Play()
{
    if (!audioLoaded)
        return;

    int propertyId = Property::GetPropertyID();

    switch (audioType)
    {
    case AUDIO_PROPERTY_BGM:
    {
        rio::AudioBgm *bgm = rio::AudioMgr::instance()->getBgm(audioKey->c_str());
        bgm->setVolume(volume);
        bgm->play(loop);
        break;
    }

    case AUDIO_PROPERTY_SFX:
    {
        rio::AudioSfx *sfx = rio::AudioMgr::instance()->getSfx(audioKey->c_str());
        sfx->setVolume(volume);
        sfx->play(Property::GetParentNode().lock()->GetPosition(), loop);
        break;
    }
    }

    RIO_LOG("[AUDIO] Played %s.\n", audioKey->c_str());
}

void AudioProperty::Stop()
{
    if (!audioLoaded)
        return;

    int propertyId = Property::GetPropertyID();

    switch (audioType)
    {
    case AUDIO_PROPERTY_BGM:
    {
        rio::AudioBgm *bgm = rio::AudioMgr::instance()->getBgm(audioKey->c_str());
        bgm->stop();
        break;
    }

    case AUDIO_PROPERTY_SFX:
    {
        rio::AudioSfx *sfx = rio::AudioMgr::instance()->getSfx(audioKey->c_str());
        sfx->stop(0);
        break;
    }
    }

    RIO_LOG("[AUDIO] Stopped %s.\n", audioKey->c_str());
}

void AudioProperty::SetVolume(const f32 volume)
{
    if (!audioLoaded)
        return;

    int propertyId = Property::GetPropertyID();

    switch (audioType)
    {
    case AUDIO_PROPERTY_BGM:
    {
        rio::AudioBgm *bgm = rio::AudioMgr::instance()->getBgm(audioKey->c_str());
        bgm->setVolume(volume);
        break;
    }

    case AUDIO_PROPERTY_SFX:
    {
        rio::AudioSfx *sfx = rio::AudioMgr::instance()->getSfx(audioKey->c_str());
        sfx->setVolume(volume);
        break;
    }
    }
}