#include <string>
#include <unordered_map>

#include "imgui.h"

namespace EditorUI {
	std::string CreateFolderDialog();
	void CreateRIOeInfo();
	void CreateProjectsList(std::unordered_map<std::string, std::string>& projectsList);
}