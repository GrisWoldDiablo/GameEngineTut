#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "Hazel/Scene/Components.h"
#include "glm/gtc/type_ptr.hpp"

#include <fstream>

namespace Hazel
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		_context = context;
	}

	void SceneHierarchyPanel::OnImGuiRender(Timestep timestep)
	{
		ImGui::Begin("Scene Hierarchy");

		_context->_registry.each([&](auto entityID)
		{
			Entity entity{ entityID,_context.get() };
			DrawEntityNode(entity);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			_selectionContext = {};
		}

		// Right-Click on blank space.
		if (ImGui::BeginPopupContextWindow(0, 1, false))
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

			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("AddComponent");
			}

			if (ImGui::BeginPopup("AddComponent"))
			{
				if (ImGui::MenuItem("Camera"))
				{
					_selectionContext.AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Sprite Renderer"))
				{
					_selectionContext.AddComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((_selectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
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

	template<typename T, typename Func>
	void Hazel::SceneHierarchyPanel::DrawComponent(Entity entity, const std::string& name, Func func)
	{
		if (auto component = entity.TryGetComponent<T>(); component != nullptr)
		{
			ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed;
			if (ImGui::TreeNodeEx((void*)typeid(T).hash_code(),treeNodeFlags, name.c_str()))
			{
				bool shouldRemoveComponent = false;
				if (typeid(T).hash_code() != typeid(TransformComponent).hash_code())
				{
					ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);
					if (ImGui::Button("+", ImVec2{ 20, 20}))
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
				func(component);
				ImGui::TreePop();

				if (shouldRemoveComponent)
				{
					entity.RemoveComponent<T>();
				}
			}
		}
	}

	static void DrawVec3Controls(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGui::PushID(label.c_str());

		ImGui::Columns(2, "vec3", false);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 0.0f });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

#pragma region ValueX
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
#pragma endregion

#pragma region ValueY
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
#pragma endregion

#pragma region ValueZ
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f ,0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		if (ImGui::Button("Z", buttonSize))
		{
			values.z = resetValue;
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
#pragma endregion

#pragma region ResetButton
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.0f, 0.0f, 0.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 1.0f , 0.75f, 0.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.0f, 0.85f, 0.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 1.0f, 0.75f, 0.0f, 1.0f });
		if (ImGui::Button("R", buttonSize))
		{
			ImGui::OpenPopup("reset");
		}
		ImGui::PopStyleColor(4);

		if (ImGui::BeginPopup("reset"))
		{
			ImGui::Text("Reset %s?", label.c_str());
			ImGui::Separator();
			if (ImGui::Button("Yes"))
			{
				values = { resetValue, resetValue, resetValue };
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
#pragma endregion


		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
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
			if (ImGui::InputText(":Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}
#pragma endregion

#pragma region TransformComponent
		DrawComponent<TransformComponent>(entity, "Transform", [&](TransformComponent* component)
		{
			DrawVec3Controls("Position", component->Position);

			glm::vec3 rotation = glm::degrees(component->Rotation);
			DrawVec3Controls("Rotation", rotation);
			component->Rotation = glm::radians(rotation);

			DrawVec3Controls("Scale", component->Scale, 1.0f);
		});
#pragma endregion

#pragma region SpriteRendererComponent
		DrawComponent<SpriteRendererComponent>(entity, "Sprite Renderer", [](SpriteRendererComponent* component)
		{
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
				if (ImGui::DragFloat("Size", &orthographicSize))
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
			std::string result;
			std::ifstream inputFileStream(component->Instance->GetClassFilePath(), std::ios::in);
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
}
