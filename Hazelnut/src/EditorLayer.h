#pragma once
#include "Hazel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Hazel/Events/KeyEvent.h"
#include "Hazel/Events/MouseEvent.h"

#include "Hazel/Renderer/EditorCamera.h"

#include "imgui/imgui.h"
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

		void OnUpdate() override;
		void OnImGuiRender() override;
		void OnEvent(Event& event) override;

	private:
		bool OnKeyPressed(KeyPressedEvent& event);

		bool NewScene(const std::string& newSceneName = _kNewSceneName);
		void OpenScene();
		void SaveSceneAs();

	private:
		void DrawToolbar();
		void DrawFileMenu();
		void DrawSceneViewport();
		void DrawStats();
		void DrawTools();
		void SafetyShutdownCheck();
		void CalculateFPS();

		void AddTooltip(const std::string& tooltipMessage);

	private:
		Color _clearColor = { 0.13f, 0.13f, 0.13f, 1.0f };

		Ref<Framebuffer> _framebuffer;
		Ref<Texture2D> _unwrapTexture;

		glm::vec2 _sceneViewportSize = { 0,0 };

		bool _isSceneViewportFocused = false;
		bool _isSceneViewportHovered = false;

		// Basics
		EditorCamera _editorCamera;
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
		int _previousGizmoType = -1;
		bool _hasStoredPreviousGizmoType = false;

		// Tools
		bool _isDemoWidowOpen = false;

		// Hover Timer
		float _timeSpentHovering = 0;

		// Toolbar Icons
		Ref<Texture2D> _panIconTexture;
		Ref<Texture2D> _eyeIconTexture;
		Ref<Texture2D> _nothingGizmoIconTexture;
		Ref<Texture2D> _positionGizmoIconTexture;
		Ref<Texture2D> _rotationGizmoIconTexture;
		Ref<Texture2D> _scaleGizmoIconTexture;
		Ref<Texture2D> _localGizmoIconTexture;
		Ref<Texture2D> _globalGizmoIconTexture;
	};
}
