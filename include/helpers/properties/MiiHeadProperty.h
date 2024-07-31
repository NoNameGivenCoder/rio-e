#ifndef MIIHEADPROPERTYHELPER_H
#define MIIHEADPROPERTYHELPER_H

#include <nn/ffl.h>
#include <helpers/properties/Property.h>
#include <Shader.h>
#include <helpers/properties/map/CameraProperty.h>
#include <math/rio_MathTypes.h>
#include <helpers/editor/EditorTypes.h>

class Shader;

class MiiHeadProperty : public Property
{
public:
    template <typename T>
    struct InitArg
    {
        FFLCharModelDesc desc;
        const T *data;
        u16 index;
    };

    enum FFLExpressionFlag
    {
        FFL_EXPRESSION_FLAG_NORMAL = 1 << 0,
        FFL_EXPRESSION_FLAG_SMILE = 1 << 1,
        FFL_EXPRESSION_FLAG_ANGER = 1 << 2,
        FFL_EXPRESSION_FLAG_SORROW = 1 << 3,
        FFL_EXPRESSION_FLAG_SURPRISE = 1 << 4,
        FFL_EXPRESSION_FLAG_BLINK = 1 << 5,
        FFL_EXPRESSION_FLAG_OPEN_MOUTH = 1 << 6,
        FFL_EXPRESSION_FLAG_HAPPY = 1 << 7,
        FFL_EXPRESSION_FLAG_ANGER_OPEN_MOUTH = 1 << 8,
        FFL_EXPRESSION_FLAG_SORROW_OPEN_MOUTH = 1 << 9,
        FFL_EXPRESSION_FLAG_SURPRISE_OPEN_MOUTH = 1 << 10,
        FFL_EXPRESSION_FLAG_BLINK_OPEN_MOUTH = 1 << 11,
        FFL_EXPRESSION_FLAG_WINK_LEFT = 1 << 12,
        FFL_EXPRESSION_FLAG_WINK_RIGHT = 1 << 13,
        FFL_EXPRESSION_FLAG_WINK_LEFT_OPEN_MOUTH = 1 << 14,
        FFL_EXPRESSION_FLAG_WINK_RIGHT_OPEN_MOUTH = 1 << 15,
        FFL_EXPRESSION_FLAG_LIKE = 1 << 16,
        FFL_EXPRESSION_FLAG_LIKE_WINK_RIGHT = 1 << 17,
        FFL_EXPRESSION_FLAG_FRUSTRATED = 1 << 18
    };

    EnumInfo expressionEnumFlags[FFL_EXPRESSION_MAX] = {
        {"Normal", FFL_EXPRESSION_FLAG_NORMAL},
        {"Smile", FFL_EXPRESSION_FLAG_SMILE},
        {"Anger", FFL_EXPRESSION_FLAG_ANGER},
        {"Sorrow", FFL_EXPRESSION_FLAG_SORROW},
        {"Surprise", FFL_EXPRESSION_FLAG_SURPRISE},
        {"Blink", FFL_EXPRESSION_FLAG_BLINK},
        {"Open Mouth", FFL_EXPRESSION_FLAG_OPEN_MOUTH},
        {"Happy", FFL_EXPRESSION_FLAG_HAPPY},
        {"Anger Open Mouth", FFL_EXPRESSION_FLAG_ANGER_OPEN_MOUTH},
        {"Sorrow Open Mouth", FFL_EXPRESSION_FLAG_SORROW_OPEN_MOUTH},
        {"Surprise Open Mouth", FFL_EXPRESSION_FLAG_SURPRISE_OPEN_MOUTH},
        {"Blink Open Mouth", FFL_EXPRESSION_FLAG_BLINK_OPEN_MOUTH},
        {"Wink Left", FFL_EXPRESSION_FLAG_WINK_LEFT},
        {"Wink Right", FFL_EXPRESSION_FLAG_WINK_RIGHT},
        {"Wink Left Open Mouth", FFL_EXPRESSION_FLAG_WINK_LEFT_OPEN_MOUTH},
        {"Wink Right Open Mouth", FFL_EXPRESSION_FLAG_WINK_RIGHT_OPEN_MOUTH},
        {"Like", FFL_EXPRESSION_FLAG_LIKE},
        {"Like Wink Right", FFL_EXPRESSION_FLAG_LIKE_WINK_RIGHT},
        {"Frustrated", FFL_EXPRESSION_FLAG_FRUSTRATED}};

    typedef InitArg<FFLStoreData> InitArgStoreData;
    typedef InitArg<FFLMiddleDB> InitArgMiddleDB;

    using Property::Property;
    ~MiiHeadProperty();

    void Start() override;
    void Update() override;
    void CreatePropertiesMenu() override;

    void Load(YAML::Node node) override;
    YAML::Node Save() override;

    void SetExpression(FFLExpressionFlag pExpressionFlag)
    {
        mCharModelDesc.expressionFlag = pExpressionFlag;
        FFLInitCharModelCPUStep(&mCharModel, &mCharModelSource, &mCharModelDesc);
        FFLInitCharModelGPUStep(&mCharModel);
    };

    void SetStoreData(FFLStoreData pStoreData)
    {
        mCharModelSource.pBuffer = &pStoreData;
        FFLInitCharModelCPUStep(&mCharModel, &mCharModelSource, &mCharModelDesc);
        FFLInitCharModelGPUStep(&mCharModel);

        GetAdditionalData();
    };

    FFLExpressionFlag GetExpression() { return (FFLExpressionFlag)(mCharModelDesc.expressionFlag); };
    FFLStoreData GetStoreData() { return mStoreData; };
    std::string GetMiiName() { return mMiiName; };

private:
    std::string mMiiDataFile = "";

    FFLStoreData mStoreData;
    FFLCharModel mCharModel;
    FFLCharModelDesc mCharModelDesc;
    FFLCharModelSource mCharModelSource;
    FFLAdditionalInfo mAdditionalInfo;

    std::string mMiiName = "";

    Shader mShader;
    const Shader *mpShader;

    CameraProperty *mainCameraProperty;

    void LoadStoreData();
    void DrawOpa();
    void DrawXlu();
    void GetAdditionalData();
};

#endif // MIIHEADPROPERTYHELPER_H