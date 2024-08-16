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
    std::vector<std::tuple<float, float, float>> normal;
    std::vector<std::tuple<float, float>> texCoord;
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
                RIO_LOG("[CONVERSIONMGR] Vert size doesn't equal four. %d\n", tokens.size());
            }

            float x, y, z;

            x = std::stof(tokens[1]);
            y = std::stof(tokens[2]);
            z = std::stof(tokens[3]);

            pos.push_back(std::make_tuple(x, y, z));
        }
        else if (tokens[0] == "vt")
        {
            if (tokens.size() != 3 || tokens.size() != 4)
            {
                RIO_LOG("[CONVERSIONMGR] Tex coord size doesn't equal 3 or 4. %d\n", tokens.size());
            }
            float u = std::stof(tokens[1]);
            float v = 1.0f - std::stof(tokens[2]);
            texCoord.push_back(std::make_tuple(u, v));
        }
        else if (tokens[0] == "vn")
        {
            if (tokens.size() != 4)
            {
                RIO_LOG("[CONVERSIONMGR] Normal size doesn't equal four. %d\n", tokens.size());
            }

            float nx = std::stof(tokens[1]);
            float ny = std::stof(tokens[2]);
            float nz = std::stof(tokens[3]);
            normal.push_back(std::make_tuple(nx, ny, nz));
        }
        else if (tokens[0] == "f")
        {
            assert(tokens.size() == 4); // Expecting 3 elements after "f"
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

    std::vector<Vertex> vertices;
    std::vector<int> indices;

    for (const auto &face : faces)
    {
        for (const auto &[posIdx, texCoordIdx, normalIdx] : {std::get<0>(face), std::get<1>(face), std::get<2>(face)})
        {
            // Handle cases where an index is -1 (i.e., missing data)
            auto posVal = posIdx != -1 ? pos[posIdx] : std::make_tuple(0.0f, 0.0f, 0.0f);
            auto texVal = texCoordIdx != -1 ? texCoord[texCoordIdx] : std::make_tuple(0.0f, 0.0f);
            auto normVal = normalIdx != -1 ? normal[normalIdx] : std::make_tuple(0.0f, 0.0f, 0.0f);

            // Create the vertex
            Vertex vertex(posVal, texVal, normVal);

            auto it = std::find(vertices.begin(), vertices.end(), vertex);
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

    returnResult.vertices = vertices;
    returnResult.indices = indices;

    RIO_LOG("Faces Count: %d\n", faces.size());
    RIO_LOG("Vertices Count: %d\n", vertices.size());
    RIO_LOG("Indices Count: %d\n", indices.size());

    return returnResult;
}

void OBJToRioModel(std::string fileName)
{
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