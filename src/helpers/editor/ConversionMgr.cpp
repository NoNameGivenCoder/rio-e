#include <helpers/editor/ConversionMgr.h>

OBJReturnResult ReadOBJ(std::string fileName)
{
    rio::FileHandle fileHandle;
    rio::FileDevice::LoadArg arg;
    arg.path = fileName;
    u8 *fileBuffer = rio::FileDeviceMgr::instance()->getNativeFileDevice()->load(arg);

    std::string objFileData(reinterpret_cast<char *>(fileBuffer));
    std::istringstream lineStream(objFileData);

    std::vector<std::tuple<float, float, float>> pos;
    std::vector<std::tuple<float, float>> texCoord;
    std::vector<std::tuple<float, float, float>> normal;
    std::vector<std::tuple<std::tuple<int, int, int>, std::tuple<int, int, int>, std::tuple<int, int, int>>> faces;

    std::string line;

    int counter = 0;
    while (std::getline(lineStream, line))
    {
        counter += 1;

        std::istringstream tokenStream(line);
        std::vector<std::string> tokens;
        std::string token;

        while (tokenStream >> token)
            tokens.push_back(token);

        if (tokens[0] == "v")
        {
            if (tokens.size() != 4)
            {
                RIO_LOG("[CONVERSIONMGR] Vertex size does not equal four!\n");
            }
            pos.emplace_back(std::make_tuple(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3])));
        }
        else if (tokens[0] == "vt")
        {
            if (tokens.size() != 3 && tokens.size() != 4)
            {
                RIO_LOG("[CONVERSIONMGR] U or V doesn't equal four or three! %d\n", tokens.size());
            }

            float u = std::stof(tokens[1]);
            float v = 1.0f - std::stof(tokens[2]);

            texCoord.emplace_back(std::make_tuple(u, v));
        }
        else if (tokens[0] == "vn")
        {
            if (tokens.size() != 4)
            {
                RIO_LOG("[CONVERSIONMGR] VN size does not equal four!\n");
            }

            float nx = std::stof(tokens[1]);
            float ny = std::stof(tokens[2]);
            float nz = std::stof(tokens[3]);
            normal.push_back(std::make_tuple(nx, ny, nz));
        }
        else if (tokens[0] == "f")
        {
            if (tokens.size() != 4)
            {
                RIO_LOG("[CONVERSIONMGR] Face size does not equal four!\n");
            }
            std::tuple<int, int, int> v1, v2, v3;

            auto parseFace = [](const std::string &face) -> std::tuple<int, int, int>
            {
                std::istringstream faceStream(face);
                std::string part;
                std::vector<int> indices;

                while (std::getline(faceStream, part, '/'))
                {
                    indices.push_back(part.empty() ? -1 : std::stoi(part) - 1);
                }

                // Fill missing parts with -1 (if they exist)
                while (indices.size() < 3)
                {
                    indices.push_back(-1);
                }

                return std::make_tuple(indices[0], indices[1], indices[2]);
            };

            v1 = parseFace(tokens[1]);
            v2 = parseFace(tokens[2]);
            v3 = parseFace(tokens[3]);

            faces.push_back(std::make_tuple(v1, v2, v3));
        }
    }

    std::vector<rio::mdl::res::Vertex> vertices;
    std::vector<int> indices;

    for (const auto &face : faces)
    {
        for (const auto &[posIdx, texCoordIdx, normalIdx] : {std::get<0>(face), std::get<1>(face), std::get<2>(face)})
        {
            rio::BaseVec3f positionVal = {std::get<0>(pos[posIdx]), std::get<1>(pos[posIdx]), std::get<2>(pos[posIdx])};
            rio::BaseVec2f texCoordVal = {std::get<0>(texCoord[texCoordIdx]), std::get<1>(texCoord[texCoordIdx])};
            rio::BaseVec3f normalVal = {std::get<0>(normal[normalIdx]), std::get<1>(normal[normalIdx]), std::get<2>(normal[normalIdx])};

            rio::mdl::res::Vertex vertex = rio::mdl::res::Vertex({positionVal, texCoordVal, normalVal});

            auto it = std::find_if(vertices.begin(), vertices.end(), [&vertex](const rio::mdl::res::Vertex &v)
                                   { return areVerticesEqual(v, vertex); });

            if (it == vertices.end())
            {
                vertices.push_back(vertex);
                indices.push_back(vertices.size() - 1);
            }
            else
            {
                indices.push_back(std::distance(vertices.begin(), it));
            }
        }
    }

    OBJReturnResult returnResult;

    returnResult.indices = indices;
    returnResult.vertices = vertices;

    RIO_LOG("Vertices Count: %d\n", vertices.size());
    RIO_LOG("Indices Count: %d\n", indices.size());

    return returnResult;
}

std::vector<Material> ReadMTL(std::string fileName)
{
    rio::FileHandle fileHandle;
    rio::FileDevice::LoadArg arg;
    arg.path = fileName;
    u8 *fileBuffer = rio::FileDeviceMgr::instance()->getNativeFileDevice()->load(arg);

    std::string mtlFileReadData(reinterpret_cast<char *>(fileBuffer));
    std::istringstream lineStream(mtlFileReadData);
    std::string line;

    std::vector<Material> mat;

    Material material;
    while (std::getline(lineStream, line))
    {
        std::istringstream tokenStream(line);
        std::vector<std::string> tokens;
        std::string token;

        if (line.empty())
            continue;

        while (tokenStream >> token)
            tokens.push_back(token);

        if (tokens[0] == "newmtl")
        {
            if (!material.name.empty())
            {
                mat.push_back(material);
            }

            material = Material();
            material.name = tokens[1];
        }

        if (tokens[0] == "map_Kd")
        {
            Texture texture = Texture();
            texture.name = tokens[1];
            texture.samplerName = "texture1";
            material.textures.emplace_back(texture);
        }
    }

    if (!material.name.empty())
    {
        mat.push_back(material);
    }

    return mat;
}

bool areVerticesEqual(const rio::mdl::res::Vertex &v1, const rio::mdl::res::Vertex &v2)
{
    return (v1.pos.x == v2.pos.x && v1.pos.y == v2.pos.y && v1.pos.z == v2.pos.z &&
            v1.tex_coord.x == v2.tex_coord.x && v1.tex_coord.y == v2.tex_coord.y &&
            v1.normal.x == v2.normal.x && v1.normal.y == v2.normal.y && v1.normal.z == v2.normal.z);
}

u32 align(u32 x, u32 y)
{
    return ((x - 1) | (y - 1)) + 1;
}

void SaveRioModel(const Model &model, bool endian, std::string fileName)
{
    u8 meshesCount = model.meshes.size();
    u8 materialsCount = model.materials.size();

    u32 meshesPos = 0x20;

    u32 vtxBufsPos = meshesPos;
    vtxBufsPos += 0x38 * meshesCount;
    vtxBufsPos = align(vtxBufsPos, 0x40);

    u32 idxBufsPos = vtxBufsPos;
    for (const auto &mesh : model.meshes)
    {
        idxBufsPos = align(idxBufsPos, 0x40);
        idxBufsPos += 0x20 * mesh.vertices.size();
    }
    idxBufsPos = align(idxBufsPos, 0x20);

    u32 materialsPos = idxBufsPos;
    for (const auto &mesh : model.meshes)
    {
        materialsPos = align(materialsPos, 0x20);
        materialsPos += 4 * mesh.indices.size();
    }

    u32 texturesPos = materialsPos;
    texturesPos += 0x80 * materialsCount;

    u32 stringsPos = texturesPos;
    for (const auto &material : model.materials)
        stringsPos += 0x48 * material.textures.size();

    u32 fileSize = stringsPos;

    for (const auto &material : model.materials)
    {
        fileSize += material.name.size() + 1;
        fileSize += material.shaderName.size() + 1;
        for (const auto &texture : material.textures)
        {
            fileSize += texture.name.size() + 1;
            fileSize += texture.samplerName.size() + 1;
        }
    }

    RIO_LOG("File Size: %d\n", fileSize);

    std::vector<u8> data;

    pack_string(data, "riomodel");
    pack_int(data, 0x01000000, false);
    pack_int(data, fileSize, false);

    if (data.size() != 0x10)
    {
        RIO_LOG("Wrong data size! 0x10\n");
        return;
    }

    pack_int(data, meshesPos - 0x10, false);
    pack_int(data, meshesCount, false);

    if (data.size() != 0x18)
    {
        RIO_LOG("Wrong data size! 0x18\n");
    }

    pack_int(data, materialsPos - 0x18, false);
    pack_int(data, materialsCount, false);

    if (data.size() != meshesPos)
    {
        RIO_LOG("Data size doesn't equal meshesPos!\n");
    }

    u32 curPos = meshesPos;
    u32 curVtxBufPos = vtxBufsPos;
    u32 curIdxBufPos = idxBufsPos;

    for (const auto &mesh : model.meshes)
    {
        // ------
        u32 vtxCount = mesh.vertices.size();
        u32 idxCount = mesh.indices.size();

        curVtxBufPos = align(curVtxBufPos, 0x40);

        pack_int(data, curVtxBufPos - curPos, false);
        curPos += 4;
        pack_int(data, vtxCount, false);
        curPos += 4;

        curVtxBufPos += 0x20 * vtxCount;
        // ------

        // ------
        curIdxBufPos = align(curIdxBufPos, 0x20);
        pack_int(data, curIdxBufPos - curPos, false);
        curPos += 4;
        pack_int(data, idxCount, false);
        curPos += 4;
        curIdxBufPos += 4 * idxCount;
        // ------

        pack_float(data, mesh.scale.x, false);
        pack_float(data, mesh.scale.y, false);
        pack_float(data, mesh.scale.z, false);
        curPos += 4 * 3;

        pack_float(data, mesh.rotate.x, false);
        pack_float(data, mesh.rotate.y, false);
        pack_float(data, mesh.rotate.z, false);
        curPos += 4 * 3;

        pack_float(data, mesh.translate.x, false);
        pack_float(data, mesh.translate.y, false);
        pack_float(data, mesh.translate.z, false);
        curPos += 4 * 3;

        pack_int(data, mesh.materialIdx, false);
        curPos += 4;
    }

    u32 padSize = align(curPos, 0x40) - curPos;
    add_padding(data, curPos, 0x40);
    curPos += padSize;

    if (curPos != vtxBufsPos)
    {
        RIO_LOG("Current position does not equal vtxBufsPos!\n");
    }

    for (const auto &mesh : model.meshes)
    {
        u32 padSize = align(curPos, 0x40) - curPos;
        add_padding(data, curPos, 0x40);
        curPos += padSize;

        for (const auto &vertex : mesh.vertices)
        {
            // Serialize vertex data into the buffer
            pack_float(data, vertex.pos.x, false);
            pack_float(data, vertex.pos.y, false);
            pack_float(data, vertex.pos.z, false);

            pack_float(data, vertex.tex_coord.x, false);
            pack_float(data, vertex.tex_coord.y, false);

            pack_float(data, vertex.normal.x, false);
            pack_float(data, vertex.normal.y, false);
            pack_float(data, vertex.normal.z, false);
        }

        curPos += 0x20 * mesh.vertices.size();
    }

    if (curPos != curVtxBufPos)
    {
        RIO_LOG("Current position does not equal curVtxBufPos\n");
    }

    padSize = align(curPos, 0x20) - curPos;
    add_padding(data, curPos, 0x20);
    curPos += padSize;

    if (curPos != idxBufsPos)
    {
        RIO_LOG("Current position does not equal idxBufsPos\n");
    }

    for (const auto &mesh : model.meshes)
    {
        padSize = align(curPos, 0x20) - curPos;
        add_padding(data, curPos, 0x20);
        curPos += padSize;
        pack_indices(data, mesh.indices, false);
        curPos += 4 * mesh.indices.size();
    }

    if (curPos != curIdxBufPos)
    {
        RIO_LOG("Current position does not equal curIdxBufPos\n");
    }

    if (curPos != materialsPos)
    {
        RIO_LOG("Current position does not equal materialsPos\n");
    }

    if (curPos != data.size())
    {
        RIO_LOG("Current position does not equal data.size\n");
    }

    curPos = materialsPos;
    u32 curTexturePos = texturesPos;
    u32 curStringPos = stringsPos;

    for (const auto &material : model.materials)
    {
        u32 materialNameLen = material.name.size() + 1;
        u32 shaderNameLen = material.shaderName.size() + 1;
        u32 textureCount = material.textures.size();

        pack_int(data, curStringPos - curPos, false);
        curPos += 4;
        pack_int(data, materialNameLen, false);
        curPos += 4;
        curStringPos += materialNameLen;

        pack_int(data, curStringPos - curPos, false);
        curPos += 4;
        pack_int(data, shaderNameLen, false);
        curPos += 4;
        curStringPos += shaderNameLen;

        for (const auto &texture : material.textures)
        {
            curStringPos += texture.name.length() + 1;
            curStringPos += texture.samplerName.length() + 1;
        }

        pack_int(data, curTexturePos - curPos, false);
        curPos += 4;
        pack_int(data, textureCount, false);
        curPos += 4;
        curTexturePos += 0x48 * textureCount;

        data.resize(data.size() + 16, 0x00);
        curPos += 16;

        u32 flags = 0;

        if (material.isVisible)
            flags |= 1;

        pack_uint16(data, flags, false);
        curPos += 2;

        u32 renderFlags = 0;

        if (material.isTranslucent)
            renderFlags |= rio::mdl::res::Material::TRANSLUCENT;

        if (material.depthTestEnable)
            renderFlags |= rio::mdl::res::Material::DEPTH_TEST_ENABLE;

        if (material.depthWriteEnable)
            renderFlags |= rio::mdl::res::Material::DEPTH_WRITE_ENABLE;

        if (material.blendEnable)
            renderFlags |= rio::mdl::res::Material::BLEND_ENABLE;

        if (material.alphaTestEnable)
            renderFlags |= 0x0008;

        if (material.colorMaskR)
            renderFlags |= rio::mdl::res::Material::COLOR_MASK_R;

        if (material.colorMaskG)
            renderFlags |= rio::mdl::res::Material::COLOR_MASK_G;

        if (material.colorMaskB)
            renderFlags |= rio::mdl::res::Material::COLOR_MASK_B;

        if (material.colorMaskA)
            renderFlags |= rio::mdl::res::Material::COLOR_MASK_A;

        if (material.stencilTestEnable)
            renderFlags |= rio::mdl::res::Material::STENCIL_TEST_ENABLE;

        if (material.polygonOffsetEnable)
            renderFlags |= rio::mdl::res::Material::POLYGON_OFFSET_ENABLE;

        if (material.polygonOffsetPointLineEnable)
            renderFlags |= rio::mdl::res::Material::POLYGON_OFFSET_POINT_LINE_ENABLE;

        pack_uint16(data, renderFlags, false);
        curPos += 2;

        pack_int(data, material.depthFunc, false);
        curPos += 4;

        pack_int(data, material.cullingMode, false);
        curPos += 4;

        pack_int(data, material.blendFactorSrcRGB, false);
        curPos += 4;

        pack_int(data, material.blendFactorSrcA, false);
        curPos += 4;

        pack_int(data, material.blendFactorDstRGB, false);
        curPos += 4;

        pack_int(data, material.blendFactorDstA, false);
        curPos += 4;

        pack_int(data, material.blendEquationRGB, false);
        curPos += 4;

        pack_int(data, material.blendEquationA, false);
        curPos += 4;

        pack_float(data, material.blendConstantColor.r / 255, false);
        pack_float(data, material.blendConstantColor.g / 255, false);
        pack_float(data, material.blendConstantColor.b / 255, false);
        pack_float(data, material.blendConstantColor.a / 255, false);
        curPos += 4 * 4;

        pack_int(data, material.alphaTestFunc, false);
        curPos += 4;

        pack_int(data, material.alphaTestRef, false);
        curPos += 4;

        pack_int(data, material.stencilTestFunc, false);
        curPos += 4;

        pack_int(data, material.stencilTestRef, false);
        curPos += 4;

        pack_int(data, material.stencilTestMask, false);
        curPos += 4;

        pack_int(data, material.stencilOpFail, false);
        curPos += 4;

        pack_int(data, material.stencilOpZFail, false);
        curPos += 4;

        pack_int(data, material.stencilOpZPass, false);
        curPos += 4;

        pack_int(data, material.polygonMode, false);
        curPos += 4;
    }

    if (curPos != texturesPos)
    {
        RIO_LOG("Current position doesn't equal texturesPos!!\n");
    }

    curStringPos = stringsPos;

    for (const auto &material : model.materials)
    {
        curStringPos += material.name.size() + 1 + material.shaderName.size() + 1;

        for (const auto &texture : material.textures)
        {
            u32 textureNameLen = texture.name.size() + 1;
            u32 samplerNameLen = texture.samplerName.size() + 1;

            pack_int(data, curStringPos - curPos, false);
            curPos += 4;
            pack_int(data, textureNameLen, false);
            curPos += 4;
            curStringPos += textureNameLen;

            pack_int(data, curStringPos - curPos, false);
            curPos += 4;
            pack_int(data, samplerNameLen, false);
            curPos += 4;
            curStringPos += samplerNameLen;

            pack_int(data, texture.magFilter, false);
            curPos += 4;

            pack_int(data, texture.minFilter, false);
            curPos += 4;

            pack_int(data, texture.mipFilter, false);
            curPos += 4;

            pack_int(data, texture.maxAniso, false);
            curPos += 4;

            pack_int(data, texture.wrapX, false);
            curPos += 4;

            pack_int(data, texture.wrapY, false);
            curPos += 4;

            pack_int(data, texture.wrapZ, false);
            curPos += 4;

            pack_float(data, texture.borderColor.r / 255, false);
            pack_float(data, texture.borderColor.g / 255, false);
            pack_float(data, texture.borderColor.b / 255, false);
            pack_float(data, texture.borderColor.a / 255, false);
            curPos += 4 * 4;

            pack_int(data, texture.minLOD, false);
            curPos += 4;

            pack_int(data, texture.maxLOD, false);
            curPos += 4;

            pack_int(data, texture.LODBias, false);
            curPos += 4;
        }
    }

    if (curPos != curTexturePos)
    {
        RIO_LOG("Current position doesn't equal current texture position!");
    }

    if (curPos != stringsPos)
    {
        RIO_LOG("Current position doesn't equal current strings position!");
    }

    for (const auto &material : model.materials)
    {
        data.insert(data.end(), material.name.begin(), material.name.end());
        data.push_back('\0');
        data.insert(data.end(), material.shaderName.begin(), material.shaderName.end());
        data.push_back('\0');

        curPos += material.name.size() + 1 + material.shaderName.size() + 1;

        for (const auto &texture : material.textures)
        {
            data.insert(data.end(), texture.name.begin(), texture.name.end());
            data.push_back('\0');
            data.insert(data.end(), texture.samplerName.begin(), texture.samplerName.end());
            data.push_back('\0');

            curPos += texture.name.size() + 1;
            curPos += texture.samplerName.size() + 1;
        }
    }

    if (curPos != curStringPos)
    {
        RIO_LOG("Current position doesn't equal current string position!");
    }

    if (curPos != fileSize)
    {
        RIO_LOG("Current position doesn't equal filesize!");
    }

    if (curPos != data.size())
    {
        RIO_LOG("Current position doesn't equal data.size()!");
    }

    rio::FileHandle fileHandle;
    rio::FileDeviceMgr::instance()->getNativeFileDevice()->open(&fileHandle, fileName + "_LE.rmdl", rio::FileDevice::FILE_OPEN_FLAG_WRITE);
    rio::FileDeviceMgr::instance()->getNativeFileDevice()->write(&fileHandle, data.data(), data.size());
    rio::FileDeviceMgr::instance()->getNativeFileDevice()->close(&fileHandle);
}

std::unique_ptr<Model> OBJToRioModel(std::string objFileName, std::string mtlFileName)
{
    OBJReturnResult returnResult = ReadOBJ(objFileName);

    Mesh mesh;
    mesh.vertices = returnResult.vertices;
    mesh.indices = returnResult.indices;
    mesh.materialIdx = 0;

    // TODO: read from .mtl file.

    auto model = std::make_unique<Model>();
    model->meshes = {mesh};
    model->materials = ReadMTL(mtlFileName);

    return std::move(model);
}

size_t ConvertGtxToRtx(u8 *fileBuffer, std::vector<u8> &fillBuffer)
{
    if (!fileBuffer)
        return 0;

    GFDFile gfd;
    size_t fileSize = gfd.load(fileBuffer);

    // Not equal to one
    if (gfd.mTextures.size() != 1)
        return 0;

    GX2Texture texture = gfd.mTextures[0];

    GX2CalcSurfaceSizeAndAlignment(&texture.surface);

    if (!&texture)
        return 0;

    GX2TexturePrintInfo(&texture);

    rio::NativeTexture2D native_texture;

    native_texture._footer.magic = 0x5101382D;
    native_texture._footer.version = 0x01000000;

    native_texture.compMap = texture.compSel;
    native_texture.surface.width = texture.surface.width;
    native_texture.surface.height = texture.surface.height;
    native_texture.surface.mipLevels = std::max((int)(texture.surface.numMips), 1);
    native_texture.surface.format = rio::TextureFormat(texture.surface.format);
    native_texture.surface.imageSize = texture.surface.imageSize;
    native_texture.surface.image = texture.surface.imagePtr;
    native_texture.surface._imageOffset = 0x80;
    native_texture.surface.mipmapSize = texture.surface.mipSize;

    if (native_texture.surface.mipLevels > 1)
    {
        assert(texture.surface.mipOffset[0] == native_texture.surface.imageSize);

        for (int i = 0; i < 13; ++i)
        {
            native_texture.surface.mipLevelOffset[i] = texture.surface.mipOffset[i];
        }

        native_texture.surface.mipLevelOffset[0] = 0;
        native_texture.surface.mipmaps = texture.surface.mipPtr;
        native_texture.surface._mipmapsOffset = native_texture.surface._imageOffset + native_texture.surface.imageSize;
    }
    else
    {
        for (int i = 0; i < 13; ++i)
        {
            native_texture.surface.mipLevelOffset[i] = 0;
        }

        native_texture.surface.mipmaps = nullptr;
        native_texture.surface._mipmapsOffset = 0;
    }

    switch (native_texture.surface.format)
    {
    case rio::TEXTURE_FORMAT_R8_UNORM:
        native_texture.surface.nativeFormat.internalformat = 0x8229;
        native_texture.surface.nativeFormat.format = 0x1903;
        native_texture.surface.nativeFormat.type = 0x1401;
        break;

    case rio::TEXTURE_FORMAT_R8_UINT:
        native_texture.surface.nativeFormat.internalformat = 0x8232;
        native_texture.surface.nativeFormat.format = 0x8D94;
        native_texture.surface.nativeFormat.type = 0x1401;
        break;

    case rio::TEXTURE_FORMAT_R8_SNORM:
        native_texture.surface.nativeFormat.internalformat = 0x8F94;
        native_texture.surface.nativeFormat.format = 0x1903;
        native_texture.surface.nativeFormat.type = 0x1400;
        break;

    case rio::TEXTURE_FORMAT_R8_SINT:
        native_texture.surface.nativeFormat.internalformat = 0x8231;
        native_texture.surface.nativeFormat.format = 0x8D94;
        native_texture.surface.nativeFormat.type = 0x1400;
        break;

    case rio::TEXTURE_FORMAT_R8_G8_UNORM:
        native_texture.surface.nativeFormat.internalformat = 0x822B;
        native_texture.surface.nativeFormat.format = 0x8227;
        native_texture.surface.nativeFormat.type = 0x1401;
        break;

    case rio::TEXTURE_FORMAT_R8_G8_UINT:
        native_texture.surface.nativeFormat.internalformat = 0x8238;
        native_texture.surface.nativeFormat.format = 0x8228;
        native_texture.surface.nativeFormat.type = 0x1401;
        break;

    case rio::TEXTURE_FORMAT_R8_G8_SNORM:
        native_texture.surface.nativeFormat.internalformat = 0x8F95;
        native_texture.surface.nativeFormat.format = 0x8227;
        native_texture.surface.nativeFormat.type = 0x1400;
        break;

    case rio::TEXTURE_FORMAT_R8_G8_SINT:
        native_texture.surface.nativeFormat.internalformat = 0x8237;
        native_texture.surface.nativeFormat.format = 0x8228;
        native_texture.surface.nativeFormat.type = 0x1400;
        break;

    case rio::TEXTURE_FORMAT_R5_G6_B5_UNORM:
        native_texture.surface.nativeFormat.internalformat = 0x8D62;
        native_texture.surface.nativeFormat.format = 0x1907;
        native_texture.surface.nativeFormat.type = 0x8363;
        break;

    case rio::TEXTURE_FORMAT_R5_G5_B5_A1_UNORM:
        native_texture.surface.nativeFormat.internalformat = 0x8057;
        native_texture.surface.nativeFormat.format = 0x1908;
        native_texture.surface.nativeFormat.type = 0x8034;
        break;

    case rio::TEXTURE_FORMAT_R4_G4_B4_A4_UNORM:
        native_texture.surface.nativeFormat.internalformat = 0x8056;
        native_texture.surface.nativeFormat.format = 0x1908;
        native_texture.surface.nativeFormat.type = 0x8033;
        break;

    case rio::TEXTURE_FORMAT_R8_G8_B8_A8_UNORM:
        native_texture.surface.nativeFormat.internalformat = 0x8058;
        native_texture.surface.nativeFormat.format = 0x1908;
        native_texture.surface.nativeFormat.type = 0x1401;
        break;

    case rio::TEXTURE_FORMAT_R8_G8_B8_A8_UINT:
        native_texture.surface.nativeFormat.internalformat = 0x8D7C;
        native_texture.surface.nativeFormat.format = 0x8D99;
        native_texture.surface.nativeFormat.type = 0x1401;
        break;

    case rio::TEXTURE_FORMAT_R8_G8_B8_A8_SNORM:
        native_texture.surface.nativeFormat.internalformat = 0x8F97;
        native_texture.surface.nativeFormat.format = 0x1908;
        native_texture.surface.nativeFormat.type = 0x1400;
        break;

    case rio::TEXTURE_FORMAT_R8_G8_B8_A8_SINT:
        native_texture.surface.nativeFormat.internalformat = 0x8D8E;
        native_texture.surface.nativeFormat.format = 0x8D99;
        native_texture.surface.nativeFormat.type = 0x1400;
        break;

    case rio::TEXTURE_FORMAT_R8_G8_B8_A8_SRGB:
        native_texture.surface.nativeFormat.internalformat = 0x8C43;
        native_texture.surface.nativeFormat.format = 0x1908;
        native_texture.surface.nativeFormat.type = 0x1401;
        break;

    case rio::TEXTURE_FORMAT_R10_G10_B10_A2_UNORM:
        native_texture.surface.nativeFormat.internalformat = 0x8059;
        native_texture.surface.nativeFormat.format = 0x1908;
        native_texture.surface.nativeFormat.type = 0x8368;
        break;

    case rio::TEXTURE_FORMAT_R10_G10_B10_A2_UINT:
        native_texture.surface.nativeFormat.internalformat = 0x906F;
        native_texture.surface.nativeFormat.format = 0x8D99;
        native_texture.surface.nativeFormat.type = 0x8368;
        break;

    case rio::TEXTURE_FORMAT_BC1_UNORM:
        native_texture.surface.nativeFormat.internalformat = 0x83F1;
        native_texture.surface.nativeFormat.format = 0;
        native_texture.surface.nativeFormat.type = 0;
        break;

    case rio::TEXTURE_FORMAT_BC1_SRGB:
        native_texture.surface.nativeFormat.internalformat = 0x8C4D;
        native_texture.surface.nativeFormat.format = 0;
        native_texture.surface.nativeFormat.type = 0;
        break;

    case rio::TEXTURE_FORMAT_BC2_UNORM:
        native_texture.surface.nativeFormat.internalformat = 0x83F2;
        native_texture.surface.nativeFormat.format = 0;
        native_texture.surface.nativeFormat.type = 0;
        break;

    case rio::TEXTURE_FORMAT_BC2_SRGB:
        native_texture.surface.nativeFormat.internalformat = 0x8C4E;
        native_texture.surface.nativeFormat.format = 0;
        native_texture.surface.nativeFormat.type = 0;
        break;

    case rio::TEXTURE_FORMAT_BC3_UNORM:
        native_texture.surface.nativeFormat.internalformat = 0x83F3;
        native_texture.surface.nativeFormat.format = 0;
        native_texture.surface.nativeFormat.type = 0;
        break;

    case rio::TEXTURE_FORMAT_BC3_SRGB:
        native_texture.surface.nativeFormat.internalformat = 0x8C4F;
        native_texture.surface.nativeFormat.format = 0;
        native_texture.surface.nativeFormat.type = 0;
        break;

    case rio::TEXTURE_FORMAT_BC4_UNORM:
        native_texture.surface.nativeFormat.internalformat = 0x8DBB;
        native_texture.surface.nativeFormat.format = 0;
        native_texture.surface.nativeFormat.type = 0;
        break;

    case rio::TEXTURE_FORMAT_BC4_SNORM:
        native_texture.surface.nativeFormat.internalformat = 0x8DBC;
        native_texture.surface.nativeFormat.format = 0;
        native_texture.surface.nativeFormat.type = 0;
        break;

    case rio::TEXTURE_FORMAT_BC5_UNORM:
        native_texture.surface.nativeFormat.internalformat = 0x8DBD;
        native_texture.surface.nativeFormat.format = 0;
        native_texture.surface.nativeFormat.type = 0;
        break;

    case rio::TEXTURE_FORMAT_BC5_SNORM:
        native_texture.surface.nativeFormat.internalformat = 0x8DBE;
        native_texture.surface.nativeFormat.format = 0;
        native_texture.surface.nativeFormat.type = 0;
        break;
    }

    size_t bufferSize = native_texture.surface._imageOffset + native_texture.surface.imageSize;
}