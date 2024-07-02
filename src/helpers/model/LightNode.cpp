#include <helpers/model/LightNode.h>
#include <rio.h>
#include <gpu/rio_Drawer.h>
#include <misc/rio_MemUtil.h>
#include <gfx/rio_Color.h>
#include <gfx/rio_PrimitiveRenderer.h>

LightNode::LightNode(rio::Color4f pLightColor, rio::Vector3f pLightPos, rio::Vector3f pLightScale, LightViewType pLightView, LightPrimitiveType pLightPrimitive, f32 pSphereRadius)
{
    mLightBlock.light_color = pLightColor;
    mLightBlock.light_pos = pLightPos;

    mViewType = pLightView;
    mPrimitiveType = pLightPrimitive;
    mLightScale = pLightScale;

    if (mPrimitiveType == LIGHT_NODE_SPHERE)
        mSphereRadius = pSphereRadius;

    RIO_LOG("New LightNode created.\n");
}

void LightNode::Draw()
{
    if (mViewType != LIGHT_NODE_VISIBLE)
        return;

    rio::PrimitiveRenderer::instance()->begin();
    switch (mViewType)
    {
    case 0:
    {
        rio::PrimitiveRenderer::CubeArg cubeArg;
        cubeArg.setCenter(mLightBlock.light_pos);
        cubeArg.setColor(mLightBlock.light_color);
        cubeArg.setSize(mLightScale);
        cubeArg.setCornerAndSize(mLightBlock.light_pos, mLightScale);

        rio::PrimitiveRenderer::instance()->drawCube(cubeArg);
        break;
    }
    case 1:
        rio::PrimitiveRenderer::instance()->drawSphere8x16(mLightBlock.light_pos, mSphereRadius, mLightBlock.light_color);
        break;
    }
    rio::PrimitiveRenderer::instance()->end();
}