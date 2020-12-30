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
		if (auto tranformComponent = entity.TryGetComponent<TransformComponent>(); tranformComponent != nullptr)
		{
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			{
				auto& transform = tranformComponent->Transform;
				auto newTransformX = transform[3][0];
				auto newTransformY = transform[3][1];
				auto newTransformZ = transform[3][2];
				ImGui::PushItemWidth(50.0f);
				ImGui::DragFloat("X", &newTransformX, 0.25f);
				ImGui::SameLine();
				ImGui::DragFloat("Y", &newTransformY, 0.25f);
				ImGui::SameLine();
				ImGui::DragFloat("Z", &newTransformZ, 0.01f, -0.999f, 1.0f);
				ImGui::PopItemWidth();
				if (newTransformX != transform[3][0] ||
					newTransformY != transform[3][1] ||
					newTransformZ != transform[3][2])
				{
					transform[3] = { newTransformX, newTransformY, newTransformZ, transform[3][3] };
					_context->SortSpriteRendererGroup(true);
				}

				ImGui::TreePop();
			}
		}
#pragma endregion

#pragma region SpriteRendererComponent
		if (auto spriteRendererComponent = entity.TryGetComponent<SpriteRendererComponent>(); spriteRendererComponent != nullptr)
		{
			if (ImGui::TreeNodeEx((void*)typeid(SpriteRendererComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Sprite Renderer"))
			{
				auto& color = spriteRendererComponent->Color;
				auto& newColor = color;
				if (ImGui::ColorEdit4("Color", newColor.GetValuePtr(), ImGuiColorEditFlags_InputRGB))
				{
					color = newColor;
				}

				ImGui::TreePop();
			}
		}
#pragma endregion
	}
}
