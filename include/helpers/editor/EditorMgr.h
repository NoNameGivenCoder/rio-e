#include <string>
#include <helpers/common/Node.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gpu/rio_RenderBuffer.h>
#include <gpu/rio_RenderTarget.h>

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

    std::string currentAssetsEditorDirectory = "/";
    std::string assetsWindowString = "Assets (/)";

    std::shared_ptr<Node> selectedNode = nullptr;

    rio::RenderTargetColor mColorTarget;
    rio::RenderTargetDepth mDepthTarget;
    rio::RenderTargetColor mItemIDTarget;
    rio::RenderBuffer mRenderBuffer;
    rio::Texture2D *mpColorTexture, *mpItemIDTexture, *mpDepthTexture;

private:
    static EditorMgr *mInstance;
    bool mInitialized = false;
    ImGuiIO io;

   

    u8 *mpItemIDReadBuffer;
    u8 *mpItemIDClearBuffer;
};
