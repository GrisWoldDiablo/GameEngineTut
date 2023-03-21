#include "hzpch.h"
#include "ContentBrowserPanel.h"
#include "Hazel/Utils/PlatformUtils.h"
#include "Utils/EditorResourceManager.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Hazel
{
	// TODO move to ImGui Utils
	static bool Button(const std::string& label, bool isEnabled = true)
	{
		if (!isEnabled)
		{
			ImGui::PushDisabled();
		}

		const bool wasButtonPressed = ImGui::Button(label.c_str());

		if (!isEnabled)
		{
			ImGui::PopDisabled();
		}

		return wasButtonPressed;
	}

	ContentBrowserPanel::ContentBrowserPanel()
		: _baseDirectory(Project::GetAssetDirectory()),
		_currentDirectory(_baseDirectory) {}

	void ContentBrowserPanel::OnImGuiRender()
	{
		const float footer = ImGui::GetFrameHeightWithSpacing();
		ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar);
		const auto panelSize = ImGui::GetContentRegionAvail();
		const float panelHeight = panelSize.y - footer;
		if (panelHeight > 0.0f && ImGui::BeginChild(ImGui::GetID("Folders"), ImVec2(panelSize.x * 0.25f, panelHeight), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar))
		{
			LoopDirectory(_baseDirectory);
			ImGui::EndChild();
		}

		ImGui::SameLine();
		const auto uv0 = ImVec2(0.0f, 1.0f);
		const auto uv1 = ImVec2(1.0f, 0.0f);
		static constexpr float THUMBNAIL_SIZE_MIN = 56.0f;
		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		auto columnCount = (int)(panelWidth / cellSize);
		columnCount = columnCount < 1 ? 1 : columnCount;

		if (panelHeight > 0.0f && ImGui::BeginChild(ImGui::GetID("FolderContent"), ImVec2(panelWidth, panelHeight), false, ImGuiWindowFlags_NoMove))
		{
			if (!std::filesystem::exists(_currentDirectory))
			{
				_currentDirectory = _baseDirectory;
			}

			const auto drawnElements = [&](bool isThumbnails)
			{
				for (auto& directoryEntry : std::filesystem::directory_iterator(_currentDirectory))
				{
					ImGui::TableNextColumn();
					const auto& path = directoryEntry.path();
					auto filenameString = path.filename().string();
					ImGui::PushID(filenameString.c_str());

					Ref<Texture2D> icon = directoryEntry.is_directory() ? Utils::ERM::GetTexture(Utils::Icon_Folder) : Utils::ERM::GetTexture(Utils::Icon_File);

					const auto elementActions = [&]
					{
						if (ImGui::BeginDragDropSource())
						{
							const wchar_t* itemPath = path.c_str();
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
								FileDialogs::ExecuteOpenFile(path.string().c_str());
								// TODO Logic based on file extension.
							}
						}

						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
						{
							ImGui::OpenPopup("Popup");
						}

						if (ImGui::BeginPopup("Popup"))
						{
							if (ImGui::MenuItem("Open Containing Folder"))
							{
								FileDialogs::ExecuteOpenFile(_currentDirectory.string().c_str());
								ImGui::CloseCurrentPopup();
							}

							if (!directoryEntry.is_directory())
							{
								ImGui::Separator();
								if (ImGui::MenuItem("Open Externally"))
								{
									FileDialogs::ExecuteOpenFile(path.string().c_str());
									ImGui::CloseCurrentPopup();
								}
							}

							ImGui::EndPopup();
						}
					};

					if (isThumbnails)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
						ImGui::ImageButton(icon->GetRawID(), {thumbnailSize, thumbnailSize}, uv0, uv1);
						ImGui::PopStyleColor();

						elementActions();

						ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + cellSize);
						ImGui::TextWrapped(filenameString.c_str());
						ImGui::PopTextWrapPos();
					}
					else
					{
						auto maxHeight = ImGui::GetTextLineHeight();
						ImGui::Image(icon->GetRawID(), {maxHeight, maxHeight}, uv0, uv1);
						ImGui::SameLine();
						ImGui::Selectable(filenameString.c_str());

						elementActions();

						if (!directoryEntry.is_directory())
						{
							ImGui::SameLine();
							ImGui::Text("Size %.2f KB", std::filesystem::file_size(path) / 1000.0f);
						}
					}

					ImGui::PopID();
				}
			};

			const bool isThumbnails = THUMBNAIL_SIZE_MIN < thumbnailSize;
			if (isThumbnails)
			{
				if (ImGui::BeginTable("FolderContent", columnCount, ImGuiTableFlags_SizingFixedSame))
				{
					drawnElements(isThumbnails);
					ImGui::EndTable();
				}
			}
			else
			{
				drawnElements(isThumbnails);
			}
			ImGui::EndChild();
		}

		ImGui::PushItemWidth(-1.0f);
		ImGui::SliderFloat("##Thumbnail", &thumbnailSize, THUMBNAIL_SIZE_MIN - 1.0f, 512.0f, "Icon Size");
		ImGui::PopItemWidth();

		if (ImGui::BeginMenuBar())
		{
			if (Button("^", _currentDirectory != _baseDirectory))
			{
				_currentDirectory = _currentDirectory.parent_path();
			}

			ImGui::Text(std::filesystem::relative(_currentDirectory, Project::GetProjectDirectory()).string().c_str());
			ImGui::EndMenuBar();
		}

		// TODO Status bar.
		ImGui::End();
	}

	bool ContentBrowserPanel::ContainDirectory(const std::filesystem::path& currentPath)
	{
		auto isDirectory = [](const std::filesystem::directory_entry& folderElement)
		{
			return folderElement.is_directory();
		};

		return std::ranges::any_of(std::filesystem::directory_iterator(currentPath), isDirectory);
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

			directories.emplace_back(folderElement, ContainDirectory(folderElement.path()));
		}

		for (const auto& [folderElement, hasDirectory] : directories)
		{
			const auto& directoryPath = folderElement.path();
			auto directoryName = directoryPath.filename().string();

			auto treeNodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

			if (!hasDirectory)
			{
				treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
			}

			if (_currentDirectory == directoryPath)
			{
				treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
			}

			const bool isExpanded = ImGui::TreeNodeEx(directoryPath.string().c_str(), treeNodeFlags, directoryName.c_str());
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
