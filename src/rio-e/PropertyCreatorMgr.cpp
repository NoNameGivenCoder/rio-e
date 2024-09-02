#include "rio-e/PropertyCreatorMgr.h"

namespace rioe
{
	PropertyCreatorMgr* PropertyCreatorMgr::mInstance = nullptr;

	bool PropertyCreatorMgr::createSingleton()
	{
		if (mInstance)
			return false;

		mInstance = new PropertyCreatorMgr();

		return true;
	}

	bool PropertyCreatorMgr::destorySingleton()
	{
		if (!mInstance)
			return false;

		delete mInstance;
		mInstance = nullptr;

		return true;
	}

	std::vector<std::string> PropertyCreatorMgr::GetAvaliableProperties()
	{
		std::vector<std::string> result;

		for (const auto& str : mCreators)
			result.emplace_back(str.first);

		return result;
	}

	std::unique_ptr<Property> PropertyCreatorMgr::CreateProperty(const std::string& type) {
		auto it = mCreators.find(type);
		if (it != mCreators.end()) {
			RIO_LOG("[PropertyCreatorMgr] Created property: %s\n", type.c_str());
			return it->second();
		}
		RIO_LOG("[PropertyCreatorMgr] Unknown property: %s\n", type.c_str());
		return nullptr;
	}
}