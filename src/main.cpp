#include <RootTask.h>
#include <rio.h>
#include <helpers/common/NodeMgr.h>
#include <helpers/common/FFLMgr.h>
#include <helpers/editor/EditorMgr.h>

static const rio::InitializeArg cInitializeArg = {
    .window = {
#if RIO_IS_DESKTOP
        .resizable = true,
        .gl_major = 4,
        .gl_minor = 3,
#endif // RIO_IS_DESKTOP
    }};

int main(int argc, char *argv[])
{
    // Initialize RIO with root task
    if (!rio::Initialize<RootTask>(cInitializeArg))
        return -1;

    // Main loop
    EditorMgr::createSingleton();
    NodeMgr::createSingleton();
    FFLMgr::createSingleton();
    StringMgr::createSingleton();
    rio::EnterMainLoop();

    // Exit RIO
    rio::Exit();
    EditorMgr::destorySingleton();
    NodeMgr::destorySingleton();
    FFLMgr::destorySingleton();
    StringMgr::destorySingleton();

    return 0;
}
