#pragma once

namespace Hazel
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

	private:
		std::filesystem::path _baseDirectory;
		std::filesystem::path _currentDirectory;

		bool ContainDirectory(const std::filesystem::path& currentPath);
		void LoopDirectory(const std::filesystem::path& currentPath);
	};
}
