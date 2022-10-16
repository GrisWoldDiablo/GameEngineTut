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

		void EditRuntimeRigidbody(Entity entity, bool shouldZeroedVelocity = false);
		Entity GetSelectedEntity() const { return _selectedEntity; }
		void SetSelectedEntity(Entity entity) { CleanUpComponents(_selectedEntity); _selectedEntity = entity; }
		void SetShouldKeepPlaying(bool value);
		bool GetShouldKeepPlaying() const { return _shouldKeepPlaying; }

	private:
		void DrawSceneName();
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		void UpdateComponents(Entity entity);
		template<typename T>
		void AddComponentMenu();
		void CleanUpComponents(Entity entity);

	private:
		Ref<Scene> _scene;
		Entity _selectedEntity;
		Entity _lockedEntity;
		bool _isDebug = false;
		Weak<AudioSource> _previousAudioSource;
		bool _shouldKeepPlaying = false;
	};
}
