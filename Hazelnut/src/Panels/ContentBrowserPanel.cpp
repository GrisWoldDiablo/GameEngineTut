#include "hzpch.h"
#include "ContentBrowserPanel.h"

#include <imgui/imgui.h>

namespace Hazel
{
	// TODO change once we have projects.
	static const std::filesystem::path sAssetsPath = "assets";

	ContentBrowserPanel::ContentBrowserPanel()
		:_currentDirectory(sAssetsPath)
	{}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		auto currentPath = _currentDirectory.string();
		if (ImGui::Button(currentPath.c_str()))
		{
			if (_currentDirectory != std::filesystem::path(sAssetsPath))
			{
				_currentDirectory = _currentDirectory.parent_path();
			}
		}

		ImGui::Separator();
		for (auto& directoryEntry : std::filesystem::directory_iterator(_currentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, sAssetsPath);
			auto relativePathString = relativePath.string();
			auto filenameString = relativePath.filename().string();

			if (directoryEntry.is_directory())
			{
				if (ImGui::Button(filenameString.c_str()))
				{
					_currentDirectory /= path.filename();
				}
			}
			else
			{
				if (ImGui::Button(filenameString.c_str()))
				{
					// TODO Logic based on file extension.
				}
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
				{
					char buffer[256];
					memset(buffer, 0, sizeof(buffer));
					strcpy_s(buffer, sizeof(buffer), path.string().c_str());
					ImGui::SetDragDropPayload("PNG_IMAGE", &buffer, sizeof(buffer));
					ImGui::Text("Dragging %s", filenameString.c_str());
					ImGui::EndDragDropSource();
				}
			}
		}

		ImGui::End();
	}
}
