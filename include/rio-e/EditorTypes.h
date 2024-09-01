#pragma once

#include <string>
#include <filesystem>

#include "rio.h"
#include "yaml-cpp/yaml.h"

namespace rioe
{
	namespace EditorTypes
	{
		class Project
		{
		public:
			std::string projectName;
			std::string defaultScene;

			std::filesystem::path filePath;

			std::string editorVersion = "1.0.0";
		};

		class Scene
		{
		public:
			std::string sceneName;
		};

		Project GetProjectData(std::filesystem::path projectPath);
	}
}