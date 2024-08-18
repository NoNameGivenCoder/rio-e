#include <string>
#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include <rio.h>
#include <filedevice/rio_FileDeviceMgr.h>

typedef std::unordered_map<std::string, std::string> StringDictionary;

class StringMgr
{
public:
    static bool createSingleton();
    static bool destorySingleton();

    static inline StringMgr *instance() { return mInstance; };

    void LoadStrings(std::string fileName, std::string dictionaryID);

    inline StringDictionary GetAllStrings(std::string dictionaryID) { return mAllStrings[dictionaryID]; };

    std::string GetString(std::string dictionaryID, std::string stringKey);

private:
    friend class EditorMgr;

    void SaveStrings(std::string dictionaryID, std::string fileName);

    static StringMgr *mInstance;
    bool mInitialized;

    std::string mStringsFolderName = rio::FileDeviceMgr::instance()->getMainFileDevice()->getContentNativePath() + "/lang/";

    std::unordered_map<std::string, StringDictionary> mAllStrings;
};