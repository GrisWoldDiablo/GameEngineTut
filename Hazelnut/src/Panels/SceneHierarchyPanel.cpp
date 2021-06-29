#include "SceneHierarchyPanel.h"
#include "Hazel/Scene/Components.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "glm/gtc/type_ptr.hpp"

#include <fstream>
#include <filesystem>

#include "Hazel/Utils/PlatformUtils.h"

namespace Hazel
{
	template<typename T>
	static void ResetButton(const std::string& label, T& values, float resetValue, ImVec2 size)
	{
		if (ImGui::Button(label.c_str(), size))
		{
			ImGui::OpenPopup("reset");
		}

		if (ImGui::BeginPopup("reset"))
		{
			ImGui::Text("Reset %s?", label.c_str());
			ImGui::Separator();
			if (ImGui::Button("Yes"))
			{
				values = T(resetValue);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			ImGui::Text(" ");
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(Entity entity, const std::string& name, UIFunction uiFunction)
	{
		if (auto component = entity.TryGetComponent<T>(); component != nullptr)
		{
			const auto treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;

			auto contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4,4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool isTreeOpened = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();

			ImGui::PushID(name.c_str());
			bool shouldRemoveComponent = false;
			if (typeid(T) != typeid(TransformComponent))
			{
				ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
				if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
				{
					ImGui::OpenPopup("ComponentSettings");
				}

				if (ImGui::BeginPopup("ComponentSettings"))
				{
					if (ImGui::MenuItem("Remove Component"))
					{
						shouldRemoveComponent = true;
					}

					ImGui::EndPopup();
				}
			}
			ImGui::PopID();

			if (isTreeOpened)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (shouldRemoveComponent)
			{
				entity.RemoveComponent<T>();
			}
		}
	}

	template<typename T>
	static void DrawVecControls(const std::string& label, T& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		const int sizeOfVec2 = sizeof(glm::vec2);
		const int sizeOfVec3 = sizeof(glm::vec3);
		auto valuesSize = sizeof(T);

		auto& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2, "vec3", false);
		ImGui::SetColumnWidth(0, columnWidth);

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

#pragma region ResetButton
		ResetButton(label, values, resetValue, ImVec2{ columnWidth, lineHeight });
#pragma endregion

		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 0.0f });

		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		if (valuesSize == sizeOfVec2 || valuesSize == sizeOfVec3)
		{
#pragma region ValueX
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("X", buttonSize))
			{
				values[0] = resetValue;
			}
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
#pragma endregion

#pragma region ValueY
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Y", buttonSize))
			{
				values[1] = resetValue;
			}
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
#pragma endregion

			if (valuesSize == sizeOfVec3)
			{
#pragma region ValueZ
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f ,0.25f, 0.8f, 1.0f });
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
				ImGui::PushFont(boldFont);
				if (ImGui::Button("Z", buttonSize))
				{
					values[2] = resetValue;
				}
				ImGui::PopFont();
				ImGui::PopStyleColor(3);

				ImGui::SameLine();
				ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f");
				ImGui::PopItemWidth();
#pragma endregion
			}
		}
		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		_context = context;
		_selectionContext = Entity::Null;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		DrawSceneName();

		_context->_registry.each([&](auto entityID)
		{
			Entity entity{ entityID,_context.get() };
			DrawEntityNode(entity);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			_selectionContext = Entity::Null;
		}

		// Right-Click on blank space.
		if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create empty Entity"))
			{
				_context->CreateEntity("Empty Entity");
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties");
		if (_selectionContext != Entity::Null)
		{
			DrawComponents(_selectionContext);
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawSceneName()
	{
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, sizeof(buffer), _context->_name.c_str());
		if (ImGui::InputText(" : Scene", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			auto newName = std::string(buffer);
			if (!newName.empty())
			{
				_context->SetName(std::string(buffer));
			}
		}
		ImGui::Separator();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((_selectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool expanded = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			_selectionContext = entity;
		}

		bool shouldDeleteEntity = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
			{
				shouldDeleteEntity = true;
			}
			ImGui::EndPopup();
		}

		if (expanded)
		{
			ImGui::TreePop();
		}

		if (shouldDeleteEntity)
		{
			_context->DestroyEntity(entity);
			if (_selectionContext == entity)
			{
				_selectionContext = Entity::Null;
			}
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
#pragma region TagComponent
		if (auto tagComponent = entity.TryGetComponent<TagComponent>(); tagComponent != nullptr)
		{
			auto& tag = tagComponent->Tag;
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputText("##:Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}

			ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("AddComponent");
			}

			if (ImGui::BeginPopup("AddComponent"))
			{
				AddComponentMenu<CameraComponent>();
				AddComponentMenu<SpriteRendererComponent>();
				ImGui::EndPopup();
			}
			ImGui::PopItemWidth();
		}
#pragma endregion

#pragma region TransformComponent
		DrawComponent<TransformComponent>(entity, "Transform", [&](TransformComponent* component)
		{
			DrawVecControls("Position", component->Position);

			glm::vec3 rotation = glm::degrees(component->Rotation);
			DrawVecControls("Rotation", rotation);
			component->Rotation = glm::radians(rotation);

			DrawVecControls("Scale", component->Scale, 1.0f);
		});
#pragma endregion

#pragma region SpriteRendererComponent
		DrawComponent<SpriteRendererComponent>(entity, "Sprite Renderer", [](SpriteRendererComponent* component)
		{
			ImGui::Text("Sprite");
			ImGui::SameLine();
			bool isSpritePressed = false;
			if (component->Texture != nullptr)
			{
				isSpritePressed = ImGui::ImageButton((ImTextureID)(intptr_t)component->Texture->GetRendererID(), ImVec2(50.0f, 50.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 3);
			}
			else
			{
				auto& color = component->Color;
				isSpritePressed = ImGui::ColorButton("None", ImVec4(color.r, color.g, color.b, color.a), 0, ImVec2(56.0f, 56.0f));
			}

			std::string filePath;
			if (isSpritePressed)
			{
				filePath = FileDialogs::OpenFile("Sprite texture (*.png)\0*.png\0");
			}

			if (component->Texture != nullptr && ImGui::BeginPopupContextItem())
			{
				ImGui::Text("Clear Texture?");
				ImGui::Separator();
				if (ImGui::Button("Yes"))
				{
					component->Texture = nullptr;
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				ImGui::Text(" ");
				ImGui::SameLine();
				if (ImGui::Button("No"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			// Added clear right click
			/*if (ImGui::MenuItem("None"))
			{
				component->Texture = nullptr;
			}*/

			if (!filePath.empty())
			{
				// TODO Look use texture assets
				component->Texture = Texture2D::Create(filePath);
			}

			if (component->Texture != nullptr)
			{
				DrawVecControls("Tiling", component->Tiling, 1.0f);
			}

			auto& color = component->Color;
			auto& newColor = color;
			if (ImGui::ColorEdit4("Color", newColor.GetValuePtr()))
			{
				color = newColor;
			}
		});
#pragma endregion

#pragma region CameraComponent
		DrawComponent<CameraComponent>(entity, "Camera", [&](CameraComponent* component)
		{
			auto& camera = component->Camera;
			ImGui::Checkbox("Primary", &component->IsPrimary);
			const char* projectionTypeStrings[] = { "Perspective","Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Perspective:
			{
				float perspectiveFOV = glm::degrees(camera.GetPerspectiveVerticalFOV());
				if (ImGui::DragFloat("FOV", &perspectiveFOV))
				{
					camera.SetPerspectiveVerticalFOV(glm::radians(perspectiveFOV));
				}

				float perspectiveNearClip = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("NearClip", &perspectiveNearClip))
				{
					camera.SetPerspectiveNearClip(perspectiveNearClip);
				}

				float perspectiveFarClip = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("FarClip", &perspectiveFarClip))
				{
					camera.SetPerspectiveFarClip(perspectiveFarClip);
				}

			}	break;
			case SceneCamera::ProjectionType::Orthographic:
			{
				float orthographicSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Size", &orthographicSize, 0.1f))
				{
					camera.SetOrthographicSize(orthographicSize);
				}

				float orthographicNearClip = camera.GetOrthographicNearClip();
				if (ImGui::DragFloat("NearClip", &orthographicNearClip))
				{
					camera.SetOrthographicNearClip(orthographicNearClip);
				}

				float orthographicFarClip = camera.GetOrthographicFarClip();
				if (ImGui::DragFloat("FarClip", &orthographicFarClip))
				{
					camera.SetOrthographicFarClip(orthographicFarClip);
				}

			}	break;
			}

			if (ImGui::Checkbox("Fixed Aspect Ratio", &component->IsFixedAspectRatio) && !component->IsFixedAspectRatio)
			{
				camera.SetViewportSize(_context->_viewportWidth, _context->_viewportHeight);
			}
		});
#pragma endregion

#pragma region NativeScriptComponent
		DrawComponent<NativeScriptComponent>(entity, "Native Script", [](NativeScriptComponent* component)
		{
			ImGui::Checkbox("Active", &component->Instance->IsEnable);
			std::string classFilePath = component->Instance->GetClassFilePath();
			auto lastSlash = classFilePath.find("\\src\\");
			auto count = classFilePath.size() - lastSlash;
			auto fileName = classFilePath.substr(lastSlash, count);
			ImGui::SameLine();
			ImGui::TextDisabled(fileName.c_str());
			std::string result;
			std::ifstream inputFileStream(classFilePath, std::ios::in);
			if (!inputFileStream.bad())
			{
				inputFileStream.seekg(0, std::ios::end);
				result.resize(inputFileStream.tellg());
				inputFileStream.seekg(0, std::ios::beg);
				inputFileStream.read(&result[0], result.size());
				inputFileStream.close();
				ImGui::BeginChild("Source", ImVec2(ImGui::GetWindowWidth() - 50, 500), true, ImGuiWindowFlags_HorizontalScrollbar);
				ImGui::Text(result.c_str());
				ImGui::EndChild();
			}
		});
#pragma endregion
	}

	template<typename T>
	void SceneHierarchyPanel::AddComponentMenu()
	{
		auto nameId = std::string(typeid(T).name());
		nameId = nameId.erase(0, nameId.find_last_of(':') + 1);
		nameId = nameId.erase(nameId.find("Component"), nameId.length());

		if (!_selectionContext.HasComponent<T>())
		{
			if (ImGui::MenuItem(nameId.c_str()))
			{
				_selectionContext.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
		else
		{
			ImGui::TextDisabled(nameId.c_str());
		}
	}
}
