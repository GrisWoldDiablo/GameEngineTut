#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>

#include "Hazel/Scene/Components.h"
#include "glm/gtc/type_ptr.hpp"

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

		ImGui::End();

		ImGui::Begin("Properties");
		if (_selectionContext != Entity::Null)
		{
			DrawComponents(_selectionContext);
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

		if (expanded)
		{
			ImGui::TreePop();
		}
	}

	template<typename T, typename Func>
	void Hazel::SceneHierarchyPanel::DrawComponent(Entity entity, const std::string& name, Func func)
	{
		if (auto component = entity.TryGetComponent<T>(); component != nullptr)
		{
			if (ImGui::TreeNodeEx((void*)typeid(T).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, name.c_str()))
			{
				func(component);
				ImGui::TreePop();
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
			if (ImGui::InputText(":Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}
#pragma endregion

#pragma region TransformComponent
		DrawComponent<TransformComponent>(entity, "Transform", [&](TransformComponent* component)
		{
			auto& transform = component->Transform;
			auto newTransformX = transform[3][0];
			auto newTransformY = transform[3][1];
			auto newTransformZ = transform[3][2];
			ImGui::PushItemWidth(50.0f);
			ImGui::DragFloat("X", &newTransformX, 0.25f);
			ImGui::SameLine();
			ImGui::DragFloat("Y", &newTransformY, 0.25f);
			ImGui::SameLine();
			ImGui::DragFloat("Z", &newTransformZ, 0.01f);
			ImGui::PopItemWidth();
			if (newTransformX != transform[3][0] ||
				newTransformY != transform[3][1] ||
				newTransformZ != transform[3][2])
			{
				transform[3] = { newTransformX, newTransformY, newTransformZ, transform[3][3] };
				_context->SortSpriteRendererGroup(true);
			}
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
	}
}
