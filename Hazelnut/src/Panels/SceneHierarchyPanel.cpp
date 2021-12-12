#include "SceneHierarchyPanel.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/ScriptableEntity.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <filesystem>

#include "Hazel/Utils/PlatformUtils.h"

namespace Hazel
{
	extern const std::filesystem::path gAssetsPath;

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
	static void DrawFloatField(const char* label, T& value, float resetValue, const Color& color, int precision = 2)
	{
		auto boldFont = ImGui::GetIO().Fonts->Fonts[0];
		float lineHeight = boldFont->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		auto buttonSize = ImVec2{ lineHeight + 3.0f, lineHeight };

		auto buttonColor = ImVec4(color.r, color.g, color.b, color.a);
		auto buttonHoveredColor = buttonColor;
		buttonHoveredColor.x *= 1.125f;
		buttonHoveredColor.y *= 2.0f;
		buttonHoveredColor.z *= 1.333f;

		ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoveredColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColor);
		ImGui::PushFont(boldFont);
		if (ImGui::Button(label, buttonSize))
		{
			value = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		auto floatLabel = std::string("##") + label;
		std::stringstream ss;
		ss << "%." << precision << "f";
		ImGui::DragFloat(floatLabel.c_str(), &value, 0.1f, 0.0f, 0.0f, ss.str().c_str());
	}

	template<typename T>
	static void DrawVecControls(const std::string& label, T& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		int valuesSize = sizeof(T) / 4;

		ImGui::PushID(label.c_str());

		ImGui::Columns(2, label.c_str(), false);
		ImGui::SetColumnWidth(0, columnWidth);

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

#pragma region ResetButton
		ResetButton(label, values, resetValue, ImVec2{ columnWidth, lineHeight });
#pragma endregion

		ImGui::NextColumn();

#pragma region FloatField
		const char* vecLabels[4] = { {"X"}, {"Y"}, {"Z"}, {"W"}, };
		const Color vecColors[4] =
		{
			{ 0.8f, 0.1f, 0.15f, 1.0f }, // Red
			{ 0.2f, 0.7f, 0.3f, 1.0f }, // Green
			{ 0.1f ,0.25f, 0.8f, 1.0f }, // Blue
			{ 0.666f ,0.745f, 0.098f, 1.0f }, // Yellow
		};

		ImGui::PushMultiItemsWidths(valuesSize, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 0.0f });

		for (int i = 0; i < valuesSize; i++)
		{
			ImGui::SameLine();
			DrawFloatField(vecLabels[i], values[i], resetValue, vecColors[i]);
			ImGui::PopItemWidth();
		}

		ImGui::PopStyleVar();

#pragma endregion

		ImGui::Columns(1);
		ImGui::PopID();
	}

	void SceneHierarchyPanel::SetScene(const Ref<Scene>& scene)
	{
		_scene = scene;
		_selectedEntity = Entity();
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_MenuBar);

		DrawSceneName();

		_scene->_registry.each([&](auto entityID)
		{
			Entity entity{ entityID, _scene.get() };
			DrawEntityNode(entity);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			_selectedEntity = Entity();
		}

		// Right-Click on blank space.
		if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create New Entity"))
			{
				_scene->CreateEntity();
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties");
		if (_selectedEntity)
		{
			DrawComponents(_selectedEntity);
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawSceneName()
	{
		if (ImGui::BeginMenuBar())
		{
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), _scene->_name.c_str());
			if (ImGui::InputText(" : Scene", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				auto newName = std::string(buffer);
				if (!newName.empty())
				{
					_scene->SetName(std::string(buffer));
				}
			}
			ImGui::Checkbox("Debug", &_isDebug);
			ImGui::EndMenuBar();
		}
		ImGui::Separator();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		ImGuiTreeNodeFlags flags = ((_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		std::string displayName = entity.Name();
		if (_isDebug)
		{
			std::stringstream ss;
			ss << displayName << "<" << entity.GetUUID() << ">";
			displayName = ss.str();
		}

		bool expanded = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, displayName.c_str());

		if (ImGui::IsItemClicked())
		{
			_selectedEntity = entity;
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
			_scene->DestroyEntity(entity);
			if (_selectedEntity == entity)
			{
				_selectedEntity = Entity();
			}
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
#pragma region BaseComponent
		if (entity)
		{
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), entity.Name().c_str());
			if (ImGui::InputText("##:Name", buffer, sizeof(buffer)))
			{
				entity.Name() = std::string(buffer);
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
				AddComponentMenu<CircleRendererComponent>();
				AddComponentMenu<Rigidbody2DComponent>();
				AddComponentMenu<BoxCollider2DComponent>();
				AddComponentMenu<CircleCollider2DComponent>();
				ImGui::EndPopup();
			}
			ImGui::PopItemWidth();

			const char* tags[] = { "Default" }; // TODO Keep tags list somewhere else
			ImGui::Combo("##Tag", &entity.Tag(), tags, IM_ARRAYSIZE(tags));

			ImGui::PushItemWidth(-1);
			ImGui::SameLine();
			const char* layers[] = { "Default" }; // TODO Keep layers list somewhere else
			ImGui::Combo("##Layer", &entity.Layer(), layers, IM_ARRAYSIZE(layers));
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

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const auto* path = (const wchar_t*)payload->Data;
					auto fileSystemPath = gAssetsPath / path;

					if (fileSystemPath.extension() == ".png")
					{
						filePath = fileSystemPath.string();
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (isSpritePressed)
			{
				filePath = FileDialogs::OpenFile("PNG (*.png)\0*.png\0");
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

			if (!filePath.empty())
			{
				component->Texture = Texture2D::Create(filePath);
			}

			if (component->Texture != nullptr)
			{
				if (ImGui::Button("Mag Filter Toggle"))
				{
					// TODO serialize Mag Filter
					auto currentMagFilter = component->Texture->GetMagFilter();
					component->Texture->ToggleMagFilter(currentMagFilter);
				}
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

#pragma region CircleRendererComponent
		DrawComponent<CircleRendererComponent>(entity, "Circle Renderer", [](CircleRendererComponent* component)
		{
			ImGui::ColorEdit4("Color", component->Color.GetValuePtr());
			ImGui::DragFloat("Thickness", &component->Thickness, 0.025f, 0.0f, 1.0f);
			ImGui::DragFloat("Fade", &component->Fade, 0.00025f, 0.0f, 1.0f);
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
				camera.SetViewportSize(_scene->_viewportWidth, _scene->_viewportHeight);
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

#pragma region Rigidbody2DComponent
		DrawComponent<Rigidbody2DComponent>(entity, "Rigidbody 2D", [&](Rigidbody2DComponent* component)
		{
			const char* bodyType[] = { "Static","Dynamic","Kinematic" };
			const char* currentBodyType = bodyType[(int)component->Type];
			if (ImGui::BeginCombo("Type", currentBodyType))
			{
				for (int i = 0; i < 3; i++)
				{
					bool isSelected = currentBodyType == bodyType[i];
					if (ImGui::Selectable(bodyType[i], isSelected))
					{
						currentBodyType = bodyType[i];
						component->Type = (Rigidbody2DComponent::BodyType)i;
					}

					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			ImGui::Checkbox("Fixed Rotation", &component->IsFixedRotation);
		});
#pragma endregion

#pragma region BoxCollider2DComponent
		DrawComponent<BoxCollider2DComponent>(entity, "Box Collider 2D", [&](BoxCollider2DComponent* component)
		{
			DrawVecControls("Offset", component->Offset, 0.0f);
			DrawVecControls("Size", component->Size, 0.5f);
			ImGui::DragFloat("Angle", &component->Angle, 0.01f, 0.0f, 360.0f);
			ImGui::DragFloat("Density", &component->Density, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction", &component->Friction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component->Restitution, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution Threshold", &component->RestitutionThreshold, 0.01f, 0.0f);
		});
#pragma endregion

#pragma region CircleCollider2DComponent
		DrawComponent<CircleCollider2DComponent>(entity, "Circle Collider 2D", [&](CircleCollider2DComponent* component)
		{
			ImGui::DragFloat("Radius", &component->Radius, 0.01f, 0.0f, 0.0f);
			ImGui::DragFloat("Density", &component->Density, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction", &component->Friction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component->Restitution, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution Threshold", &component->RestitutionThreshold, 0.01f, 0.0f);
		});
#pragma endregion
	}

	template<typename T>
	void SceneHierarchyPanel::AddComponentMenu()
	{
		auto nameId = std::string(typeid(T).name());
		nameId = nameId.erase(0, nameId.find_last_of(':') + 1);
		nameId = nameId.erase(nameId.find("Component"), nameId.length());

		if (!_selectedEntity.HasComponent<T>())
		{
			if (ImGui::MenuItem(nameId.c_str()))
			{
				_selectedEntity.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
		else
		{
			ImGui::TextDisabled(nameId.c_str());
		}
	}
}
