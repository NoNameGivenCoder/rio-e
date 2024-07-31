#ifndef AUDIOHELPER_H
#define AUDIOHELPER_H

#include <rio.h>
#include <audio/rio_AudioMgr.h>
#include <audio/rio_AudioSrc.h>
#include <gfx/rio_Camera.h>
#include <helpers/common/Node.h>
#include <helpers/properties/Property.h>

class AudioProperty : public Property
{
public:
    enum AudioType
    {
        AUDIO_PROPERTY_BGM = 0,
        AUDIO_PROPERTY_SFX = 1
    };

    struct AudioPropertyInitArgs
    {
        std::shared_ptr<std::string> audioFile;
        AudioType audioType;
        f32 volume = 1.f;
    };

    using Property::Property;

    void Load(YAML::Node node) override;
    YAML::Node Save() override;

    void Update() override;
    void Start() override;
    void CreatePropertiesMenu() override;

    void Play();
    void Stop();
    void SetVolume(const f32 volume);

    void SetAudioFile(std::shared_ptr<std::string> pAudioFile)
    {
        audioFile = pAudioFile;
        LoadAudio();
    };

    inline void SetLoop(const bool pLoop) { loop = pLoop; };

    // Properties

private:
    void LoadAudio();

    std::shared_ptr<std::string> audioFile = std::make_shared<std::string>("");
    std::shared_ptr<std::string> audioKey = std::make_shared<std::string>("");
    AudioType audioType = AUDIO_PROPERTY_BGM;
    f32 volume = 1.0f;
    bool audioLoaded = false;
    bool loop = 0;
};

#endif // AUDIOHELPER_H