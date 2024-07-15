#ifndef LIGHTHELPER_H
#define LIGHTHELPER_H

#include <rio.h>
#include <math/rio_Vector.h>
#include <gfx/rio_Color.h>
#include <helpers/common/Node.h>
#include <gfx/mdl/rio_Model.h>

class LightNode : public Node
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

    struct LightNodeInitArgs
    {
        rio::Color4f LightColor = {1.f, 1.f, 1.f, 1.f};
        LightViewType LightView = LIGHT_NODE_INVISIBLE;
        LightPrimitiveType LightPrimitive = LIGHT_NODE_SPHERE;
        f32 LightSphereRadius = 1.f;
    };

    LightBlock mLightBlock;
    rio::UniformBlock mUniformBlock;

    using Node::Node;
    void Init(LightNodeInitArgs args);

    void Draw();

    LightBlock GetLightBlock() { return mLightBlock; };
    rio::Color4f GetLightColor() { return mLightColor; };

    void SetLightRadius(f32 pRadius) { mLightSphereRadius = pRadius; };
    void SetLightColor(rio::Color4f pLightColor) { mLightColor = pLightColor; };
    void SetLightViewType(LightViewType pLightView) { mLightViewType = pLightView; };
    void SetLightPrimitive(LightPrimitiveType pLightPrimitive) { mLightPrimitiveType = pLightPrimitive; };

private:
    bool mInitialized = false;
    LightViewType mLightViewType = LIGHT_NODE_INVISIBLE;
    LightPrimitiveType mLightPrimitiveType = LIGHT_NODE_SPHERE;
    f32 mLightSphereRadius = 1.f;
    rio::Color4f mLightColor = {1.f, 1.f, 1.f, 1.f};
};

#endif // LIGHTHELPER_H