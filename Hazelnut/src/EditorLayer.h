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

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep timestep) override;

		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;

	private:
		bool OnKeyPressed(const KeyPressedEvent& keyPressedEvent);
		bool OnMouseButtonUp(const MouseButtonUpEvent& mouseButtonReleasedEvent);
		void MousePicking();
		void OnOverlayRender() const;

		bool ClearSceneCheck() const;
		void NewScene(bool withCheck = false);
		void OpenScene();
		void OpenScene(const std::filesystem::path& path, bool withCheck = false);
		void SaveScene();
		bool SaveSceneAs(const std::filesystem::path& defaultPath);
		void SerializeScene() const;

		void NewProject();
		void OpenProject();
		void OpenProject(const std::filesystem::path& path);
		void SaveProject();
		void SetProjectStartSceneToCurrent();
		
	private:
		void DrawToolbar();
		void DrawFileMenu();
		void DrawNewProjectPopup();
		void DrawSceneViewport();
		void DrawStats();
		void DrawTools();
		void SafetyShutdownCheck();
		void CalculateFPS();

		void SetWindowTitleSceneName(const std::filesystem::path& scenePath = "") const;
		void AddTooltip(const std::string& tooltipMessage);

		void OnScenePlay();
		void OnSceneSimulate();
		void OnSceneStop();

		void DuplicateEntity();

	private:
		Color _clearColor = {0.13f, 0.13f, 0.13f, 1.0f};
		Ref<Framebuffer> _framebuffer;

		glm::vec2 _sceneViewportSize = {0.0f, 0.0f};

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
		float _updateTimerFastestElapsedMillis = FLT_MAX;
		float _updateTimerSlowestElapsedMillis = -FLT_MAX;

		Timer _imGuiTimer;
		float _imGuiTimerElapsedMillis = 0.0f;
		float _imGuiTimerFastestElapsedMillis = FLT_MAX;
		float _imGuiTimerSlowestElapsedMillis = -FLT_MAX;

		// Panels
		SceneHierarchyPanel _sceneHierarchyPanel;
		Scope<ContentBrowserPanel> _contentBrowserPanel;

		// SceneViewport
		int _gizmoType = -1;
		int _gizmoSpace = 0;
		int _previousGizmoType = -1;
		bool _hasStoredPreviousGizmoType = false;
		glm::vec2 _sceneViewportBounds[2] = {};
		Entity _hoveredEntity;

		// Tools
		bool _shouldShowPhysicsColliders = false;
		bool _isDemoWidowOpen = false;

		// New Project Popup Modal
		bool _shouldOpenNewProjectModal = false;
		std::string _newProjectName;
		std::filesystem::path _newProjectPath;

		// Hover Timer
		float _timeSpentHovering = 0;

		enum class SceneState
		{
			Edit = 0,
			Play = 1,
			Simulate = 2
		} _sceneState = SceneState::Edit;
	};
}
