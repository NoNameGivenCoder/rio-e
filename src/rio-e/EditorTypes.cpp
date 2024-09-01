#include <filesystem>

#include "rio-e/EditorTypes.h"

namespace rioe {
	namespace EditorTypes {
		Project GetProjectData(std::filesystem::path projectPath)
		{
			Project project;
			std::filesystem::path configFilePath = projectPath / "projectConfig.yaml";
			if (!std::filesystem::exists(configFilePath))
			{
				RIO_LOG("[EditorTypes] Config file: %s does not exist.\n", configFilePath.c_str());
				return project;
			}

			YAML::Node node = YAML::LoadFile(configFilePath.string());

			project.projectName = node["project"]["projectName"].as<std::string>();
			project.editorVersion = node["project"]["editorVersion"].as<std::string>();
			project.defaultScene = node["project"]["defaultScene"].as<std::string>();

			return project;
		}
	}
}
