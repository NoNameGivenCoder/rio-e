#include <helpers/common/StringMgr.h>

StringMgr *StringMgr::mInstance = nullptr;

bool StringMgr::createSingleton()
{
    if (mInstance)
        return false;

    mInstance = new StringMgr();
    mInstance->mInitialized = true;

    if (!mInstance->mInitialized)
    {
        delete mInstance;
        mInstance = nullptr;
        return false;
    }

    return true;
}

bool StringMgr::destorySingleton()
{
    if (!mInstance)
        return false;

    mInstance->mAllStrings.clear();

    delete mInstance;
    mInstance = nullptr;

    return true;
}

void StringMgr::LoadStrings(std::string fileName, std::string dictionaryID)
{
    RIO_LOG("[STRINGMGR] Loading strings from %s..\n", fileName.c_str());

    YAML::Node stringsYaml = (YAML::LoadFile(mStringsFolderName + "/" + fileName))["strings"];

    if (!stringsYaml)
        return;

    StringDictionary strDictionary;

    for (YAML::const_iterator it = stringsYaml.begin(); it != stringsYaml.end(); ++it)
        strDictionary.emplace(it->first.as<std::string>(), it->second.as<std::string>());

    mAllStrings[dictionaryID] = strDictionary;

    RIO_LOG("[STRINGMGR] Loaded %d strings with %s\n", strDictionary.size(), dictionaryID.c_str());
}

std::string StringMgr::GetString(std::string dictionaryID, std::string stringKey)
{
    if (mAllStrings[dictionaryID].empty())
        return "";

    return mAllStrings[dictionaryID][stringKey];
}

void StringMgr::SaveStrings(std::string dictionaryID, std::string fileName)
{
    YAML::Emitter outNode;

    outNode << YAML::BeginMap << YAML::Key << "strings" << YAML::BeginMap;

    for (const auto &strings : mAllStrings[dictionaryID])
        outNode << YAML::Key << strings.first << YAML::Value << strings.second;

    outNode << YAML::EndMap;

    rio::FileDevice *fileDevice = rio::FileDeviceMgr::instance()->getNativeFileDevice();
    rio::FileHandle fileHandle;

    fileDevice->open(&fileHandle, mStringsFolderName + "/" + fileName, rio::FileDevice::FILE_OPEN_FLAG_WRITE);
    fileDevice->write(&fileHandle, (u8 *)(outNode.c_str()), strlen(outNode.c_str()));
    fileDevice->close(&fileHandle);
}