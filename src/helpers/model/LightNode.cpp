#include <helpers/model/LightNode.h>
#include <rio.h>
#include <gpu/rio_Drawer.h>
#include <misc/rio_MemUtil.h>
#include <gfx/rio_Color.h>
#include <gfx/rio_PrimitiveRenderer.h>

void LightNode::Init(LightNodeInitArgs args)
{
    //__attribute__((aligned(rio::Drawer::cUniformBlockAlignment))) LightNode::LightBlock mLightBlock;

    if (mInitialized)
    {
        RIO_LOG("[LIGHTNODE] %s already initialized.\n", Node::nodeKey);
        return;
    }

    mLightViewType = args.LightView;
    mLightPrimitiveType = args.LightPrimitive;
    mLightSphereRadius = args.LightSphereRadius;
    mLightColor = args.LightColor;

    mLightBlock.light_pos = Node::GetPosition();
    mLightBlock.light_color = args.LightColor;

    mInitialized = true;
    RIO_LOG("[LIGHTNODE] %s initialized.\n", Node::nodeKey);
}

void LightNode::Draw()
{
    mLightBlock.light_pos = Node::GetPosition();

    if (mLightViewType != LIGHT_NODE_VISIBLE)
        return;

    rio::PrimitiveRenderer::instance()->begin();
    switch (mLightViewType)
    {
    case 0:
    {
        rio::PrimitiveRenderer::CubeArg cubeArg;
        cubeArg.setCenter(Node::GetPosition());
        cubeArg.setColor(mLightBlock.light_color);
        cubeArg.setSize(Node::GetScale());
        cubeArg.setCornerAndSize(Node::GetPosition(), Node::GetScale());

        rio::PrimitiveRenderer::instance()->drawCube(cubeArg);
        break;
    }
    case 1:
        rio::PrimitiveRenderer::instance()->drawSphere8x16(Node::GetPosition(), mLightSphereRadius, mLightColor);
        break;
    }
    rio::PrimitiveRenderer::instance()->end();
}