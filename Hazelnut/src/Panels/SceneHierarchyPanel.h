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

		void OnImGuiRender(Timestep timestep);

	private:
		void DrawEntityNode(Entity entity);
		template<typename T, typename Func>
		void DrawComponent(Entity entity, const std::string& name, Func func);
		void DrawComponents(Entity entity);
		
	private:
		Ref<Scene> _context;
		Entity _selectionContext;
	};
}
