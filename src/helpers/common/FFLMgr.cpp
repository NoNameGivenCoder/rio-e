#include <helpers/common/FFLMgr.h>
#include <nn/ffl.h>
#include <string>
#include <filedevice/rio_FileDeviceMgr.h>
#include <misc/rio_MemUtil.h>

FFLMgr *FFLMgr::mInstance = nullptr;

bool FFLMgr::createSingleton()
{
    if (mInstance)
        return false;

    mInstance = new FFLMgr();
    mInstance->mInitialized = true;

    if (!mInstance->mInitialized)
    {
        delete mInstance;
        mInstance = nullptr;
        return false;
    }

    return true;
}

bool FFLMgr::destorySingleton()
{
    if (!mInstance)
        return false;

    RIO_LOG("[FFLMGR] Exiting FFL..\n");

    FFLExit();

    // Free the resources and set pointers to nullptr.
    if (mInstance->mResourceDesc.pData[FFL_RESOURCE_TYPE_HIGH])
    {
        rio::MemUtil::free(mInstance->mResourceDesc.pData[FFL_RESOURCE_TYPE_HIGH]);
        // mInstance->mResourceDesc.pData[FFL_RESOURCE_TYPE_HIGH] = nullptr;
    }
    if (mInstance->mResourceDesc.pData[FFL_RESOURCE_TYPE_MIDDLE])
    {
        rio::MemUtil::free(mInstance->mResourceDesc.pData[FFL_RESOURCE_TYPE_MIDDLE]);
        // mInstance->mResourceDesc.pData[FFL_RESOURCE_TYPE_MIDDLE] = nullptr;
    }

    if (mInstance->miiBufferSize)
    {
        rio::MemUtil::free(mInstance->miiBufferSize);
    }
    delete mInstance;
    // mInstance = nullptr;
    //  mInstance->miiBufferSize = nullptr;

    return true;
}

void FFLMgr::CreateRandomMiddleDB(u16 pMiiLength)
{
    miiBufferSize = new u8[FFLGetMiddleDBBufferSize(pMiiLength)];
    FFLInitMiddleDB(&mMiddleDB, FFL_MIDDLE_DB_TYPE_RANDOM_PARAM, miiBufferSize, pMiiLength);
    FFLUpdateMiddleDB(&mMiddleDB);
    RIO_LOG("[FFLMGR] Created Random Middle DB.\n");
}

void FFLMgr::InitializeFFL()
{
    FFLInitDesc init_desc;
    init_desc.fontRegion = FFL_FONT_REGION_0;
    init_desc._c = false;
    init_desc._10 = true;

#if RIO_IS_CAFE
    FSInit();
#endif // RIO_IS_CAFE

    std::string resPath;
    resPath.resize(256);
    // Middle
    {
        FFLGetResourcePath(resPath.data(), 256, FFL_RESOURCE_TYPE_MIDDLE, false);
        {
            rio::FileDevice::LoadArg arg;
            arg.path = resPath;
            arg.alignment = 0x2000;

            u8 *buffer = rio::FileDeviceMgr::instance()->getNativeFileDevice()->tryLoad(arg);
            if (buffer == nullptr)
            {
                RIO_LOG("NativeFileDevice failed to load: %s\n", resPath.c_str());
                RIO_ASSERT(false);
                return;
            }

            mResourceDesc.pData[FFL_RESOURCE_TYPE_MIDDLE] = buffer;
            mResourceDesc.size[FFL_RESOURCE_TYPE_MIDDLE] = arg.read_size;
        }
    }
    // High
    {
        FFLGetResourcePath(resPath.data(), 256, FFL_RESOURCE_TYPE_HIGH, false);
        {
            rio::FileDevice::LoadArg arg;
            arg.path = resPath;
            arg.alignment = 0x2000;

            u8 *buffer = rio::FileDeviceMgr::instance()->getNativeFileDevice()->tryLoad(arg);
            if (buffer == nullptr)
            {
                RIO_LOG("NativeFileDevice failed to load: %s\n", resPath.c_str());
                RIO_ASSERT(false);
                return;
            }

            mResourceDesc.pData[FFL_RESOURCE_TYPE_HIGH] = buffer;
            mResourceDesc.size[FFL_RESOURCE_TYPE_HIGH] = arg.read_size;
        }
    }

    FFLResult result = FFLInitResEx(&init_desc, &mResourceDesc);
    if (result != FFL_RESULT_OK)
    {
        RIO_LOG("FFLInitResEx() failed with result: %d\n", (s32)result);
        RIO_ASSERT(false);
        return;
    }

    FFLiEnableSpecialMii(333326543);

    RIO_ASSERT(FFLIsAvailable());

    FFLInitResGPUStep();
}