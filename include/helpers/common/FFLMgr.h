#ifndef FFLHELPER_H
#define FFLHELPER_H

#include <stdio.h>
#include <string>
#include <nn/ffl.h>
#include <filedevice/rio_FileDeviceMgr.h>

class FFLMgr
{
public:
    static bool createSingleton();
    static bool destorySingleton();

    static inline FFLMgr *instance() { return mInstance; };

    void InitializeFFL();
    void CreateRandomMiddleDB(u16 pMiiLength);

    FFLStoreData GetStoreDataFromFile(std::string fileName, rio::RawErrorCode *errCode);

    FFLResolution GetGlobalResolution() { return mResolution; };

    FFLMiddleDB mMiddleDB;

private:
    static FFLMgr *mInstance;
    FFLResourceDesc mResourceDesc;
    bool mInitialized;

    void *miiBufferSize;

    FFLResolution mResolution;
};

#endif // FFLHELPER_H