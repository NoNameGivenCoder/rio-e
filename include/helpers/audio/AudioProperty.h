#ifndef AUDIOHELPER_H
#define AUDIOHELPER_H

#include <rio.h>
#include <audio/rio_AudioMgr.h>
#include <audio/rio_AudioSrc.h>
#include <gfx/rio_Camera.h>
#include <helpers/common/Node.h>
#include <helpers/common/CameraNode.h>
#include <helpers/common/Property.h>

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
        std::string audioFile;
        AudioType audioType;
        f32 volume = 1.f;
    };

    using Property::Property;

    void Load(YAML::Node node);
    void Play(const bool loop = false);
    void Update();
    void SetVolume(const f32 volume);

    void CreatePropertiesMenu();

    // Properties
    std::string audioFile;
    AudioType audioType;
    f32 volume;

private:
};

#endif // AUDIOHELPER_H