#include <cstdio>
#include <iostream>

#include "rio.h"
#include "EditorTask.h"

#include "rio-e/EditorMgr.h"
#include "rio-e/SceneMgr.h"
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

static const rio::InitializeArg cInitializeArg = {
    .window = {
#if RIO_IS_WIN
        .resizable = true,
        .gl_major = 4,
        .gl_minor = 3,
#endif // RIO_IS_WIN
    } };

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	RIO_LOG("[RIO(e)] Starting..\n");
    rioe::EditorMgr::createSingleton();
    rioe::SceneMgr::createSingleton();
    rioe::PropertyCreatorMgr::createSingleton();

    if (!rio::Initialize<EditorTask>(cInitializeArg))
        return -1;

    rio::EnterMainLoop();

    rio::Exit();
    rioe::PropertyCreatorMgr::destorySingleton();
    rioe::SceneMgr::destorySingleton();
    rioe::EditorMgr::destorySingleton();

	return 0;
}
