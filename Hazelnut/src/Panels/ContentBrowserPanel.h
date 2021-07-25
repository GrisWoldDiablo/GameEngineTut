#pragma once
#include "Hazel/Renderer/Texture.h"

namespace Hazel
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

	private:
		std::filesystem::path _currentDirectory;

		Ref<Texture2D> _folderIconTexture;
		Ref<Texture2D> _fileIconTexture;
	};
}
