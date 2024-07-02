#ifndef LIGHTHELPER_H
#define LIGHTHELPER_H

#include <rio.h>
#include <math/rio_Vector.h>
#include <gfx/rio_Color.h>

class LightNode
{
public:
    struct LightBlock
    {
        rio::Color4f light_color;
        u32 _padding_0;
        rio::Vector3f light_pos;
        u32 _padding_1;
    };

    enum LightViewType
    {
        // Light node will not be visible to camera.
        LIGHT_NODE_INVISIBLE = 0,
        // Light node will be visible to camera.
        LIGHT_NODE_VISIBLE = 1,
    };

    enum LightPrimitiveType
    {
        LIGHT_NODE_CUBE = 0,
        LIGHT_NODE_SPHERE = 1
    };

    LightNode(rio::Color4f pLightColor, rio::Vector3f pLightPos = {0, 0, 0}, rio::Vector3f pLightScale = {1, 1, 1}, LightViewType pLightView = LIGHT_NODE_INVISIBLE, LightPrimitiveType pLightPrimitive = LIGHT_NODE_SPHERE, f32 pSphereRadius = 1.f);
    void Draw();

    LightBlock GetLightBlock() { return mLightBlock; };

    void SetScale(rio::Vector3f pScale) { mLightScale = pScale; };
    void SetPos(rio::Vector3f pPos) { mLightBlock.light_pos = pPos; };
    void SetRadius(f32 pRadius) { mSphereRadius = pRadius; };

private:
    LightViewType mViewType;
    LightPrimitiveType mPrimitiveType;
    LightBlock mLightBlock;
    rio::Vector3f mLightScale;
    f32 mSphereRadius;
};

#endif // LIGHTHELPER_H