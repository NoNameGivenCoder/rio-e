#include <RootTask.h>
#include <rio.h>
#include <helpers/common/NodeMgr.h>
#include <helpers/common/FFLMgr.h>

static const rio::InitializeArg cInitializeArg = {
    .window = {
#if RIO_IS_WIN
        .resizable = true,
        .gl_major = 4,
        .gl_minor = 3,
#endif // RIO_IS_WIN
    }};

int main(int argc, char *argv[])
{
    // Initialize RIO with root task
    if (!rio::Initialize<RootTask>(cInitializeArg))
        return -1;

    // Main loop
    NodeMgr::createSingleton();
    FFLMgr::createSingleton();
    rio::EnterMainLoop();

    NodeMgr::destorySingleton();
    // Exit RIO
    rio::Exit();
    FFLMgr::destorySingleton();

    return 0;
}
