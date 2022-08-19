#include "hzpch.h"
#include "ContentBrowserPanel.h"
#include "Hazel/Core/Color.h"
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
		auto panelSize = ImGui::GetContentRegionAvail();
		if (ImGui::BeginChildFrame(ImGui::GetID("Folders"), ImVec2(panelSize.x * 0.25f, panelSize.y), ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar))
		{
			LoopDirectory(gAssetsPath);
			ImGui::EndChildFrame();
		}

		const auto uv0 = ImVec2(0.0f, 1.0f);
		const auto uv1 = ImVec2(1.0f, 0.0f);
		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;
		float panelWidth = panelSize.x * 0.75f;
		auto columnCount = (int)(panelWidth / cellSize);
		columnCount = columnCount < 1 ? 1 : columnCount;

		ImGui::SameLine();
		if (ImGui::BeginChildFrame(ImGui::GetID("FolderContent"), ImVec2(panelWidth, panelSize.y), ImGuiWindowFlags_NoMove))
		{
			if (!std::filesystem::exists(_currentDirectory))
			{
				_currentDirectory = gAssetsPath;
			}

			if (ImGui::BeginTable("FolderContent", columnCount, ImGuiTableFlags_SizingFixedSame))
			{
				for (auto& directoryEntry : std::filesystem::directory_iterator(_currentDirectory))
				{
					ImGui::TableNextColumn();
					const auto& path = directoryEntry.path();
					auto filenameString = path.filename().string();
					ImGui::PushID(filenameString.c_str());

					Ref<Texture2D> icon = directoryEntry.is_directory() ? _folderIconTexture : _fileIconTexture;

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
					ImGui::ImageButton((ImTextureID)(intptr_t)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, uv0, uv1);
					ImGui::PopStyleColor();

					if (ImGui::BeginDragDropSource())
					{
						auto relativePath = std::filesystem::relative(path, gAssetsPath);
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
						else if (FileDialogs::QuestionPopup("Do you want to open folderElement in external program?", "Open File"))
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

			ImGui::EndChildFrame();
		}

		if (ImGui::BeginMenuBar())
		{
			//ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x);
			auto currentPath = _currentDirectory.string();
			if (ImGui::Button(currentPath.c_str()))
			{
				if (_currentDirectory != std::filesystem::path(gAssetsPath))
				{
					_currentDirectory = _currentDirectory.parent_path();
				}
			}

			ImGui::PushItemWidth(-100);
			ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 56.0f, 512.0f);
			ImGui::PopItemWidth();

			ImGui::EndMenuBar();
		}

		// TODO Status bar.
		ImGui::End();
	}

	bool ContentBrowserPanel::ContainDirectory(const std::filesystem::path& currentPath)
	{
		for (auto& folderElement : std::filesystem::directory_iterator(currentPath))
		{
			if (folderElement.is_directory())
			{
				return true;
			}
		}

		return false;
	}

	void ContentBrowserPanel::LoopDirectory(const std::filesystem::path& currentPath)
	{

		std::vector<std::tuple<std::filesystem::directory_entry, bool>> directories;


		for (auto& folderElement : std::filesystem::directory_iterator(currentPath))
		{
			if (!folderElement.is_directory())
			{
				continue;
			}

			directories.push_back({ folderElement, ContainDirectory(folderElement.path()) });
		}

		for (const auto& tupleItem : directories)
		{
			const auto& folderElement = std::get<0>(tupleItem);
			const auto& directoryPath = folderElement.path();
			auto directoryName = directoryPath.filename().string();

			auto treeNodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

			bool hasDirectory = std::get<1>(tupleItem);
			if (!hasDirectory)
			{
				treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
			}

			if (_currentDirectory == directoryPath)
			{
				treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
			}

			bool isExpanded = ImGui::TreeNodeEx(directoryPath.string().c_str(), treeNodeFlags, directoryName.c_str());
			if (ImGui::IsItemClicked())
			{
				_currentDirectory = directoryPath;
			}

			if (isExpanded)
			{
				if (hasDirectory)
				{
					LoopDirectory(directoryPath);
				}

				ImGui::TreePop();
			}
		}
	}
}
