#pragma once

namespace Hazel
{
	class Project;

	class ProjectSerializer
	{
	public:
		ProjectSerializer(const Ref<Project>& project);

		bool Serialize(const std::filesystem::path& filepath) const;
		bool Deserialize(const std::filesystem::path& filepath) const;

	private:
		Ref<Project> _project;
	};
}
