#include <string>
#include <unordered_map>

#include "imgui.h"
#include "rio-e/EditorTypes.h"

namespace rioe {
	namespace ProjectsUI {
		void CreateNewProject(std::filesystem::path path);
		bool WriteFile(char* inp, std::string path);
		std::string CreateFolderDialog(const char* promptTitle);
		void CreateRIOeInfo();
		void CreateProjectsList();
	}
}