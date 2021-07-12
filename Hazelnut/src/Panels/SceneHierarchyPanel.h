#pragma once

#include "Hazel/Core/Base.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"

namespace Hazel
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;

		void SetScene(const Ref<Scene>& _scene);

		void OnImGuiRender();

		Entity GetSelectedEntity() const { return _selectedEntity; }
		void SetSelectedEntity(Entity entity) { _selectedEntity = entity; }

	private:
		void DrawSceneName();
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		template<typename T>
		void AddComponentMenu();

	private:
		Ref<Scene> _scene;
		Entity _selectedEntity;
	};
}
