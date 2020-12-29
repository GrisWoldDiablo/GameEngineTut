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
				auto newTransform = transform[3];
				if (ImGui::DragFloat3(":Position", glm::value_ptr(newTransform), 0.5f))
				{
					transform[3] = newTransform;
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
				auto newColor = color;
				if (ImGui::ColorEdit4("Color", newColor.GetValuePtr()))
				{
					color = newColor;
				}

				ImGui::TreePop();
			}
		}
#pragma endregion
	}
}
