#pragma once
#include "Hazel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Hazel/Events/KeyEvent.h"

namespace Hazel
{
	constexpr char* _kNewSceneName = "Untitled";

	class EditorLayer final : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer() = default;
		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(Timestep timestep) override;
		void OnImGuiRender(Timestep timestep) override;
		void OnEvent(Event& event) override;

	private:
		bool OnKeyPressed(KeyPressedEvent& event);

		bool NewScene(const std::string& newSceneName = _kNewSceneName);
		void OpenScene();
		void SaveSceneAs();

	private:
		void DrawFileMenu();
		void DrawSceneViewport();
		void DrawStats(Timestep timestep);
		void DrawTools();
		void SafetyShutdownCheck();
		void CalculateFPS(Timestep timestep);

	private:
		Color _clearColor = { 0.13f, 0.13f, 0.13f, 1.0f };

		Ref<Framebuffer> _framebuffer;
		Ref<Texture2D> _unwrapTexture;

		glm::vec2 _sceneViewportSize = { 0,0 };

		bool _isSceneViewportFocused = false;
		bool _isSceneViewportHovered = false;
		bool _isOnSecondCamera = false;

		Entity _squareEntity;
		Entity _mainCamera;
		Entity _secondaryCamera;

		Ref<Scene> _activeScene;

		// FPS
		int _frameCount = 0;
		int _currentFPS = 60;
		float _oneSecondCountDown = 1.0f;
		int _lowFrames = 0;
		InstrumentationTimer _updateTimer;

		// Panels
		SceneHierarchyPanel _sceneHierarchyPanel;

		// SceneViewport
		int _gizmoType = -1;
		int _gizmoSpace = 0;

		// Tools
		bool _isDemoWidowOpen = false;
	};
}
