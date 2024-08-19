#include <string>
#include <helpers/common/Node.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gpu/rio_RenderBuffer.h>
#include <gpu/rio_RenderTarget.h>
#include <filedevice/rio_FileDevice.h>
#include <filedevice/rio_FileDeviceMgr.h>
#include <fstream>
#include <filesystem>
#include <ninTexUtils/dds.h>
#include <ninTexUtils/gfd/gfdStruct.h>
#include <ninTexUtils/gx2/tcl/addrlib.h>
#include <ninTexUtils/gx2/gx2Enum.h>
#include <ninTexUtils/types.h>
#include <ninTexUtils/gfd/gfdEnum.h>
#include <ninTexUtils/util.h>
#include <map>
#include <array>
#include <cstdint>
#include <variant>
#include <algorithm>
#include <helpers/common/StringMgr.h>
#include <helpers/editor/ConversionMgr.h>

class EditorMgr
{
public:
    static bool createSingleton();
    static bool destorySingleton();

    static inline EditorMgr *instance() { return mInstance; };

    static void CreateNodePropertiesMenu();
    void CreateEditorUI();
    void BindRenderBuffer();
    void UnbindRenderBuffer();
    void SetupFrameBuffer();
    void Update();
    void MakeFontIcon(const char *characters);

    std::string currentAssetsEditorDirectory = "/";
    std::string assetsWindowString = "Assets (/)";

    std::shared_ptr<Node> mSelectedNode = nullptr;

    rio::RenderTargetColor mColorTarget;
    rio::RenderTargetDepth mDepthTarget;
    rio::RenderBuffer mRenderBuffer;
    rio::Texture2D *mpColorTexture, *mpDepthTexture;

private:
    static EditorMgr *mInstance;
    bool mInitialized = false;

    rio::FileDevice *mFileDevice;
    rio::FileHandle fileHandle;

    ImGuiIO io;

    void DrawStringsWindow();
    void DrawTexturesWindow();

    bool mStringsWindowEnabled;
    bool mStringsUnsaved = false;
    std::string mSelectedStringsID;
    std::string mSelectedStringKey;
    std::string mSelectedStringContent;
    std::string mStringFolderPath = "";
    std::string mStringWindowName = "String Viewer";
    std::vector<std::filesystem::path> mStringCachedContents;
    std::filesystem::file_time_type mStringLastWriteTime;
    StringDictionary mCurrentStringDictionary;
    void UpdateStringsDirCache();
    static int UnsavedStringCallback(ImGuiInputTextCallbackData *data) { mInstance->mStringsUnsaved = true; };

    bool mTextureWindowEnabled;
    std::string mTextureFolderPath = "";
    std::string mTextureWindowName = "Texture Viewer";
    std::filesystem::path mTextureSelected;
    std::vector<std::filesystem::path> mTextureCachedContents;
    std::unordered_map<std::string, std::unique_ptr<rio::Texture2D>> mTextures;
    std::filesystem::file_time_type mTexturesLastWriteTime;
    std::unordered_map<rio::TextureFormat, std::string> mTextureFormatMap = {
        {rio::TextureFormat::TEXTURE_FORMAT_BC1_SRGB, "BC1_SRGB"},
        {rio::TextureFormat::TEXTURE_FORMAT_BC1_UNORM, "BC1_UNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_BC2_SRGB, "BC2_SRGB"},
        {rio::TextureFormat::TEXTURE_FORMAT_BC2_UNORM, "BC2_UNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_BC3_SRGB, "BC3_SRGB"},
        {rio::TextureFormat::TEXTURE_FORMAT_BC3_UNORM, "BC3_UNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_BC4_SNORM, "BC4_SNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_BC4_UNORM, "BC4_UNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_BC5_SNORM, "BC5_SNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_BC5_UNORM, "BC5_UNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_INVALID, "INVALID"},
        {rio::TextureFormat::TEXTURE_FORMAT_R10_G10_B10_A2_UINT, "R10_G10_B10_A2_UINT"},
        {rio::TextureFormat::TEXTURE_FORMAT_R10_G10_B10_A2_UNORM, "R10_G10_B10_A2_UNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_R32_UINT, "R32_UINT"},
        {rio::TextureFormat::TEXTURE_FORMAT_R4_G4_B4_A4_UNORM, "R4_G4_B4_A4_UNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_R5_G5_B5_A1_UNORM, "R5_G5_B5_A1_UNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_R5_G6_B5_UNORM, "R5_G6_B5_UNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_R8_G8_B8_A8_SINT, "R8_G8_B8_A8_SINT"},
        {rio::TextureFormat::TEXTURE_FORMAT_R8_G8_B8_A8_SNORM, "R8_G8_B8_A8_SNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_R8_G8_B8_A8_SRGB, "R8_G8_B8_A8_SRGB"},
        {rio::TextureFormat::TEXTURE_FORMAT_R8_G8_B8_A8_UINT, "R8_G8_B8_A8_UINT"},
        {rio::TextureFormat::TEXTURE_FORMAT_R8_G8_B8_A8_UNORM, "R8_G8_B8_A8_UNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_R8_G8_SINT, "R8_G8_SINT"},
        {rio::TextureFormat::TEXTURE_FORMAT_R8_G8_SNORM, "R8_G8_SNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_R8_G8_UINT, "R8_G8_UINT"},
        {rio::TextureFormat::TEXTURE_FORMAT_R8_G8_UNORM, "R8_G8_UNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_R8_SINT, "R8_SINT"},
        {rio::TextureFormat::TEXTURE_FORMAT_R8_SNORM, "R8_SNORM"},
        {rio::TextureFormat::TEXTURE_FORMAT_R8_UINT, "R8_UINT"},
        {rio::TextureFormat::TEXTURE_FORMAT_R8_UNORM, "R8_UNORM"},
        {rio::TextureFormat::DEPTH_TEXTURE_FORMAT_R16_UNORM, "R16_UNORM"},
        {rio::TextureFormat::DEPTH_TEXTURE_FORMAT_R32_FLOAT, "R32_FLOAT"}};

    using InnerCompMap = std::map<uint16_t, std::vector<std::array<u32, 4>>>;
    using OuterCompMap = std::map<uint8_t, InnerCompMap>;

    using FourCCMap = std::map<std::string, std::array<uint16_t, 2>>;

    FourCCMap fourCCs = {
        {"DXT1", {0x031, 0x08}},
        {"DXT2", {0x032, 0x10}},
        {"DXT3", {0x032, 0x10}},
        {"DXT4", {0x033, 0x10}},
        {"DXT5", {0x033, 0x10}},
        {"ATI1", {0x034, 0x08}},
        {"BC4U", {0x034, 0x08}},
        {"BC4S", {0x234, 0x08}},
        {"ATI2", {0x035, 0x10}},
        {"BC5U", {0x035, 0x10}},
        {"BC5S", {0x235, 0x10}},
    };

    // Define the outer map
    OuterCompMap validComps = {
        {0x08, {
                   {0x001, {{0x000000ff}}},
                   {0x002, {{0x0000000f, 0x000000f0}}},
               }},
        {0x10, {
                   {0x007, {{0x000000ff, 0x0000ff00}}},
                   {0x008, {{0x0000001f, 0x000007e0, 0x0000f800}}},
                   {0x00a, {{0x0000001f, 0x000003e0, 0x00007c00, 0x00008000}}},
                   {0x00b, {{0x0000000f, 0x000000f0, 0x00000f00, 0x0000f000}}},
               }},
        {0x20, {
                   {0x019, {{0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000}}},
                   {0x01a, {{0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000}}},
               }},
    };

    template <typename Container>
    int findIndex(const Container &masks, u32 mask)
    {
        auto it = std::find(masks.begin(), masks.end(), mask);
        return (it != masks.end()) ? std::distance(masks.begin(), it) : -1;
    }

    void addToBuffer(void *data, size_t size, std::vector<u8> *vect)
    {
        size_t oldSize = vect->size();
        vect->resize(oldSize + sizeof(GFDHeader));
        std::memcpy(vect->data() + oldSize, data, sizeof(GFDHeader));
    };

    void UpdateTexturesDirCache();
    u8 *ConvertDDSToGtx();
};
