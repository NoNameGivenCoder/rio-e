#include <rio.h>
#include <gfx/mdl/res/rio_ModelData.h>
#include <gfx/mdl/res/rio_MeshData.h>
#include <gfx/mdl/res/rio_MaterialData.h>

#include <filedevice/rio_FileDeviceMgr.h>
#include <filedevice/rio_FileDevice.h>

#include <ninTexUtils/gfd/gfdStruct.h>

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <endian.h>

struct Mesh
{
    u32 materialIdx = -1;

    std::vector<rio::mdl::res::Vertex> vertices;
    std::vector<int> indices;

    rio::Vector3f scale = {1.0, 1.0, 1.0};
    rio::Vector3f rotate = {0.0, 0.0, 0.0};
    rio::Vector3f translate = {0.0, 0.0, 0.0};
};

struct OBJReturnResult
{
    std::vector<rio::mdl::res::Vertex> vertices;
    std::vector<int> indices;
};

struct Texture
{
    std::string name = "";
    std::string samplerName = "";

    rio::TexXYFilterMode magFilter = rio::TEX_XY_FILTER_MODE_LINEAR;
    rio::TexXYFilterMode minFilter = rio::TEX_XY_FILTER_MODE_LINEAR;
    rio::TexMipFilterMode mipFilter = rio::TEX_MIP_FILTER_MODE_LINEAR;
    rio::TexAnisoRatio maxAniso = rio::TEX_ANISO_1_TO_1;

    rio::TexWrapMode wrapX = rio::TEX_WRAP_MODE_CLAMP;
    rio::TexWrapMode wrapY = rio::TEX_WRAP_MODE_CLAMP;
    rio::TexWrapMode wrapZ = rio::TEX_WRAP_MODE_CLAMP;

    rio::Color4f borderColor = {0, 0, 0, 255};

    float minLOD = 0.0;
    float maxLOD = 14.0;
    float LODBias = 0.0;
};

struct Material
{
    std::string name = "";
    std::string shaderName = "";

    bool isVisible = true;

    std::vector<Texture> textures = {};
    // self.uniformVars = []
    // self.uniformBlocks = []

    bool isTranslucent = false;

    bool depthTestEnable = true;
    bool depthWriteEnable = true;
    rio::Graphics::CompareFunc depthFunc = rio::Graphics::COMPARE_FUNC_LEQUAL;

    rio::Graphics::CullingMode cullingMode = rio::Graphics::CULLING_MODE_BACK;

    bool blendEnable = true;
    rio::Graphics::BlendFactor blendFactorSrcRGB = rio::Graphics::BlendFactor::BLEND_MODE_SRC_ALPHA;
    rio::Graphics::BlendFactor blendFactorSrcA = rio::Graphics::BlendFactor::BLEND_MODE_SRC_ALPHA;
    rio::Graphics::BlendFactor blendFactorDstRGB = rio::Graphics::BlendFactor::BLEND_MODE_ONE_MINUS_SRC_ALPHA;
    rio::Graphics::BlendFactor blendFactorDstA = rio::Graphics::BlendFactor::BLEND_MODE_ONE_MINUS_SRC_ALPHA;
    rio::Graphics::BlendEquation blendEquationRGB = rio::Graphics::BlendEquation::BLEND_FUNC_ADD;
    rio::Graphics::BlendEquation blendEquationA = rio::Graphics::BlendEquation::BLEND_FUNC_ADD;
    rio::Color4f blendConstantColor = {255, 255, 255, 255};

    bool alphaTestEnable = false;
    rio::Graphics::CompareFunc alphaTestFunc = rio::Graphics::CompareFunc::COMPARE_FUNC_GREATER;
    float alphaTestRef = 0.0;

    bool colorMaskR = true;
    bool colorMaskG = true;
    bool colorMaskB = true;
    bool colorMaskA = true;

    bool stencilTestEnable = false;
    rio::Graphics::CompareFunc stencilTestFunc = rio::Graphics::CompareFunc::COMPARE_FUNC_NEVER;
    int stencilTestRef = 0;
    u8 stencilTestMask = 0xFFFFFFFF;
    rio::Graphics::StencilOp stencilOpFail = rio::Graphics::StencilOp::STENCIL_KEEP;
    rio::Graphics::StencilOp stencilOpZFail = rio::Graphics::StencilOp::STENCIL_KEEP;
    rio::Graphics::StencilOp stencilOpZPass = rio::Graphics::StencilOp::STENCIL_KEEP;

    rio::Graphics::PolygonMode polygonMode = rio::Graphics::PolygonMode::POLYGON_MODE_FILL;
    bool polygonOffsetEnable = false;
    bool polygonOffsetPointLineEnable = false;
};

struct Model
{
    std::vector<Mesh> meshes = {};
    std::vector<Material> materials = {};
};

OBJReturnResult ReadOBJ(std::string fileName);
void OBJToRioModel(std::string fileName);
bool areVerticesEqual(const rio::mdl::res::Vertex &v1, const rio::mdl::res::Vertex &v2);

u32 align(u32 x, u32 y);
inline void pack_int(std::vector<std::uint8_t> &buffer, uint32_t value, bool isBigEndian)
{
    if (isBigEndian)
        value = htobe32(value);
    else
        value = htole32(value);

    buffer.resize(buffer.size() + sizeof(value));

    std::memcpy(buffer.data() + buffer.size() - sizeof(value), &value, sizeof(value));
}

inline void pack_float(std::vector<uint8_t> &buffer, float value, bool isBigEndian)
{
    uint32_t temp;
    std::memcpy(&temp, &value, sizeof(float));
    if (isBigEndian)
    {
        temp = htobe32(temp);
    }
    else
    {
        temp = htole32(temp);
    }
    buffer.resize(buffer.size() + sizeof(temp));
    std::memcpy(buffer.data() + buffer.size() - sizeof(temp), &temp, sizeof(temp));
}

inline void pack_string(std::vector<std::uint8_t> &buffer, const std::string &str)
{
    buffer.resize(buffer.size() + str.size());
    std::memcpy(buffer.data() + buffer.size() - str.size(), str.c_str(), str.size());
}

inline void pack_uint16(std::vector<std::uint8_t> &buffer, uint16_t value, bool isBigEndian)
{
    if (isBigEndian)
        value = htobe16(value);

    buffer.resize(buffer.size() + sizeof(value));
    std::memcpy(buffer.data() + buffer.size() - sizeof(value), &value, sizeof(value));
}

inline void add_padding(std::vector<uint8_t> &buffer, std::size_t curPos, std::size_t alignment)
{
    std::size_t paddedPos = align(curPos, alignment);
    std::size_t padSize = paddedPos - curPos;
    if (padSize > 0)
    {
        buffer.resize(buffer.size() + padSize, 0); // Append zero bytes for padding
    }
}

inline void pack_indices(std::vector<uint8_t> &buffer, const std::vector<int> &indices, bool isBigEndian)
{
    for (int index : indices)
    {
        uint32_t value = static_cast<uint32_t>(index);

        // Handle endianness
        if (isBigEndian)
        {
            value = htobe32(value);
        }
        else
        {
            value = htole32(value);
        }

        // Append to buffer
        buffer.resize(buffer.size() + sizeof(value));
        std::memcpy(buffer.data() + buffer.size() - sizeof(value), &value, sizeof(value));
    }
}

size_t ConvertGtxToRtx(u8 *fileBuffer, std::vector<u8> &fillBuffer);