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
			auto& transform = entity.GetComponent<TransformComponent>().Transform;
			ImGui::DragFloat3("", glm::value_ptr(transform[3]));

			auto colorComponent = entity.TryGetComponent<SpriteRendererComponent>();
			if (colorComponent != nullptr)
			{
				ImGui::ColorEdit4("Color", colorComponent->Color.GetValuePtr());
			}
			ImGui::TreePop();
		}
	}
}
