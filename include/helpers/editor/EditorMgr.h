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

    std::string currentAssetsEditorDirectory = "/";
    std::string assetsWindowString = "Assets (/)";

    std::shared_ptr<Node> selectedNode = nullptr;

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

    void UpdateTexturesDirCache();
};
