#include <cstdio>
#include <iostream>

#include "rio.h"
#include "EditorTask.h"

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
	RIO_LOG("[RIO(e)] Starting..\n");

    if (!rio::Initialize<EditorTask>(cInitializeArg))
        return -1;

    rio::EnterMainLoop();

    rio::Exit();

	return 0;
}
