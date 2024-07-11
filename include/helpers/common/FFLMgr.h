#ifndef FFLHELPER_H
#define FFLHELPER_H

#include <stdio.h>
#include <string>
#include <nn/ffl.h>

class FFLMgr
{
public:
    static bool createSingleton();
    static bool destorySingleton();

    static inline FFLMgr *instance() { return mInstance; };

    void InitializeFFL();
    void CreateRandomMiddleDB(u16 pMiiLength);

    FFLMiddleDB mMiddleDB;

private:
    static FFLMgr *mInstance;
    FFLResourceDesc mResourceDesc;
    bool mInitialized;

    void *miiBufferSize;
};

#endif // FFLHELPER_H