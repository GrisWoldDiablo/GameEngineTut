#pragma once
#include "Hazel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"

#include "Hazel/Events/KeyEvent.h"
#include "Hazel/Events/MouseEvent.h"

#include "Hazel/Renderer/EditorCamera.h"

#include "Hazel/Core/Timer.h"

namespace Hazel
{
	inline const char* kNewSceneName = "Untitled";

	class EditorLayer final : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(const Timestep& timestep) override;

		void OnImGuiRender() override;
		void OnEvent(Event& event) override;

	private:
		bool OnKeyPressed(const KeyPressedEvent& keyPressedEvent);
		bool OnMouseButtonReleased(const MouseButtonReleasedEvent& mouseButtonReleasedEvent);
		void MousePicking();
		void OnOverlayRender();

		bool ClearSceneCheck();
		void NewScene(bool withCheck = false);
		void OpenScene();
		void OpenScene(const std::filesystem::path& path, bool withCheck = false);
		void SaveScene();

		void SerializeScene();

		void SaveSceneAs();

	private:
		void DrawToolbar();
		void DrawFileMenu();
		void DrawSceneViewport();
		void DrawStats();
		void DrawTools();
		void SafetyShutdownCheck();
		void CalculateFPS();

		void SetWindowTitleSceneName(const std::filesystem::path& scenePath = "");
		void AddTooltip(const std::string& tooltipMessage);

		void OnScenePlay();
		void OnSceneSimulate();
		void OnSceneStop();

		void DuplicateEntity();

	private:
		Color _clearColor = { 0.13f, 0.13f, 0.13f, 1.0f };
		Ref<Framebuffer> _framebuffer;

		glm::vec2 _sceneViewportSize = { 0.0f, 0.0f };

		bool _isSceneViewportFocused = false;
		bool _isSceneViewportHovered = false;

		// Basics
		EditorCamera _editorCamera;
		Ref<Scene> _activeScene;
		Ref<Scene> _editorScene;
		std::filesystem::path _editorScenePath;

		// FPS
		int _frameCount = 0;
		int _currentFPS = 60;
		float _oneSecondCountDown = 1.0f;
		int _lowFrames = 0;
		Timer _updateTimer;
		float _updateTimerElapsedMillis = 0.0f;

		// Panels
		SceneHierarchyPanel _sceneHierarchyPanel;
		ContentBrowserPanel _contentBrowserPanel;

		// SceneViewport
		int _gizmoType = -1;
		int _gizmoSpace = 0;
		int _previousGizmoType = -1;
		bool _hasStoredPreviousGizmoType = false;
		glm::vec2 _sceneViewportBounds[2] = {};
		Entity _hoveredEntity;

		// Tools
		bool _shouldShowPhysicsColliders = true; // TODO Set to false once we have save editor settings ability.
		bool _isDemoWidowOpen = false;

		// Hover Timer
		float _timeSpentHovering = 0;

		// Toolbar Icons
		enum class Icons
		{
			Pan,
			Magnifier,
			Eye,
			Nothing,
			Position,
			Rotation,
			Scale,
			Local,
			Global,
			Play,
			Stop,
			Simulate
		};

		std::map<Icons, Ref<Texture2D>> _iconTextures;

		enum class SceneState
		{
			Edit = 0,
			Play = 1,
			Simulate = 2
		} _sceneState = SceneState::Edit;

	};
}
