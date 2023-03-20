#include "hzpch.h"
#include "ProjectSerializer.h"

#include "Project.h"
#include "yaml-cpp/yaml.h"

namespace Hazel
{
	ProjectSerializer::ProjectSerializer(const Ref<Project>& project)
		: _project(project) { }

	bool ProjectSerializer::Serialize(const std::filesystem::path& filepath) const
	{
		const auto& config = _project->GetConfig();

		YAML::Emitter out;

		out << YAML::BeginMap; // Root
		out << YAML::Key << "Project" << YAML::Value;
		{
			out << YAML::BeginMap; // Project
			out << YAML::Key << "Name" << YAML::Value << config.Name;
			out << YAML::Key << "StartScene" << YAML::Value << config.StartScene.string();
			out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
			out << YAML::Key << "ScriptModulePath" << YAML::Value << config.ScriptModulePath.string();
			out << YAML::EndMap; // Project
		}
		out << YAML::EndMap; // Root

		std::ofstream fout(filepath);
		fout << out.c_str();

		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath) const
	{
		auto& config = _project->GetConfig();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException& e)
		{
			HZ_CORE_LERROR("Failed to load project file [{0}].\n\t'{1}'", filepath, e.what());
			return false;
		}

		if (!data["Project"])
		{
			return false;
		}

		const auto projectNode = data["Project"];

		config.Name = projectNode["Name"].as<std::string>();
		config.StartScene = projectNode["StartScene"].as<std::string>();
		config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
		config.ScriptModulePath = projectNode["ScriptModulePath"].as<std::string>();

		return true;
	}
}
