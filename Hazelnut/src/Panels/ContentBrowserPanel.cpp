#include "hzpch.h"
#include "ContentBrowserPanel.h"
#include "Hazel/Utils/PlatformUtils.h"

#include <imgui/imgui.h>

namespace Hazel
{
	// TODO change once we have projects.
	extern const std::filesystem::path gAssetsPath = "assets";

	ContentBrowserPanel::ContentBrowserPanel()
		:_currentDirectory(gAssetsPath)
	{
		_folderIconTexture = Texture2D::Create("Resources/Icons/ContentBrowser/FolderIcon256.png");
		_fileIconTexture = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon256.png");
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_MenuBar);

		if (ImGui::BeginMenuBar())
		{
			auto currentPath = _currentDirectory.string();
			if (ImGui::Button(currentPath.c_str()))
			{
				if (_currentDirectory != std::filesystem::path(gAssetsPath))
				{
					_currentDirectory = _currentDirectory.parent_path();
				}
			}
			ImGui::EndMenuBar();
		}

		const auto uv0 = ImVec2(0.0f, 1.0f);
		const auto uv1 = ImVec2(1.0f, 0.0f);
		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		auto columnCount = (int)(panelWidth / cellSize);
		columnCount = columnCount < 1 ? 1 : columnCount;

		if (ImGui::BeginTable("FolderContent", columnCount, ImGuiTableFlags_SizingFixedSame))
		{
			for (auto& directoryEntry : std::filesystem::directory_iterator(_currentDirectory))
			{
				ImGui::TableNextColumn();
				const auto& path = directoryEntry.path();
				auto relativePath = std::filesystem::relative(path, gAssetsPath);
				auto relativePathString = relativePath.string();
				auto filenameString = relativePath.filename().string();
				ImGui::PushID(filenameString.c_str());

				Ref<Texture2D> icon = directoryEntry.is_directory() ? _folderIconTexture : _fileIconTexture;

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
				ImGui::ImageButton((ImTextureID)(intptr_t)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, uv0, uv1);
				ImGui::PopStyleColor();

				if (ImGui::BeginDragDropSource())
				{
					const wchar_t* itemPath = relativePath.c_str();
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
					ImGui::Text(filenameString.c_str());
					ImGui::EndDragDropSource();
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (directoryEntry.is_directory())
					{
						_currentDirectory /= path.filename();
					}
					else if (FileDialogs::QuestionPopup("Do you want to open file in external program?", "Open File"))
					{
						FileDialogs::ExecuteFile(path.string().c_str());
						// TODO Logic based on file extension.
					}
				}

				float tw = ImGui::CalcTextSize(filenameString.c_str()).x;
				float offSet = (cellSize - tw - padding) / 2.0f;
				offSet = offSet < 0.0f ? 0.0f : offSet;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offSet);
				ImGui::TextWrapped(filenameString.c_str());

				ImGui::PopID();
			}
			ImGui::EndTable();
		}

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 56.0f, 512.0f);
		ImGui::SliderFloat("Padding", &padding, 0.0f, 32.0f);

		// TODO Status bar.
		ImGui::End();
	}
}
