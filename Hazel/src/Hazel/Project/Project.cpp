#include "hzpch.h"
#include "Project.h"
#include "ProjectSerializer.h"

namespace Hazel
{
	Ref<Project> Project::New(const std::filesystem::path& path)
	{
		_sActiveProject = CreateRef<Project>();
		_sActiveProject->_projectDirectory = path;
		return _sActiveProject;
	}

	Ref<Project> Project::Load(const std::filesystem::path& path)
	{
		const auto project = CreateRef<Project>();

		const ProjectSerializer serializer(project);
		if (serializer.Deserialize(path))
		{
			project->_projectDirectory = path.parent_path();
			_sActiveProject = project;
			return _sActiveProject;
		}

		return nullptr;
	}
	
	bool Project::SaveActive()
	{
		const std::filesystem::path filename = fmt::format("{0}.hproj", _sActiveProject->GetConfig().Name);
		return SaveActive(_sActiveProject->_projectDirectory / filename);
	}

	bool Project::SaveActive(const std::filesystem::path& filepath)
	{
		const ProjectSerializer serializer(_sActiveProject);
		if (serializer.Serialize(filepath))
		{
			_sActiveProject->_projectDirectory = filepath.parent_path();
			return true;
		}

		return false;
	}
}
