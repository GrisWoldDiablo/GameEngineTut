#pragma once

namespace Hazel
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

	private:
		std::filesystem::path _currentDirectory;
	};
}
