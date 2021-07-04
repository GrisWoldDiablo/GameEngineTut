#pragma once

#include "Hazel/Core/Base.h"
#include "Hazel/Core/Log.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"

namespace Hazel
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& context);

		void SetContext(const Ref<Scene>& context);

		void OnImGuiRender();

		Entity GetSelectedEntity() const { return _selectionContext; }
		void SetSelectedEntity(Entity entity) { _selectionContext = entity; }

	private:
		void DrawSceneName();
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		template<typename T>
		void AddComponentMenu();

	private:
		Ref<Scene> _context;
		Entity _selectionContext;
	};
}
