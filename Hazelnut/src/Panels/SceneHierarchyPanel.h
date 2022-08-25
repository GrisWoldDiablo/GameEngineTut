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
		void SetSelectedEntity(Entity entity) { CleanUpComponents(_selectedEntity); _selectedEntity = entity; }

	private:
		void DrawSceneName();
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		template<typename T>
		void AddComponentMenu();
		void CleanUpComponents(Entity entity);

	private:
		Ref<Scene> _scene;
		Entity _selectedEntity;
		bool _isDebug = false;
		Weak<AudioSource> _previousAudioSource;
	};
}
