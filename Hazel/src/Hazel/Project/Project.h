#pragma once

namespace Hazel
{
	struct ProjectConfig
	{
		std::string Name = "Untitled";

		std::filesystem::path StartScene;

		std::filesystem::path AssetDirectory;
		std::filesystem::path ScriptModulePath;
	};

	class Project
	{
	public:
		ProjectConfig& GetConfig() { return _config; }
		
		static Ref<Project> New(const std::filesystem::path& path);
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive();
		static bool SaveActive(const std::filesystem::path& filepath);
		
		static Ref<Project> GetActive() { return _sActiveProject; }

		static const std::filesystem::path& GetProjectDirectory()
		{
			HZ_ENSURE(_sActiveProject);
			return _sActiveProject->_projectDirectory;
		}
		
		static std::filesystem::path GetAssetDirectory()
		{
			HZ_ENSURE(_sActiveProject);
			return GetProjectDirectory() / _sActiveProject->_config.AssetDirectory;
		}

		// TODO move to asset manager.
		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{
			HZ_ENSURE(_sActiveProject);
			return GetAssetDirectory() / path;
		}

	private:
		ProjectConfig _config;
		std::filesystem::path _projectDirectory;
		
		inline static Ref<Project> _sActiveProject;
	};
}
