#include "EditorLayer.h"
#include "NativeScripts.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Utils/PlatformUtils.h"

#include "ImGuizmo.h"
#include "Hazel/Math/Math.h"

namespace Hazel
{
	EditorLayer::EditorLayer()
		: Layer("Hazel Editor")
	{}

	void EditorLayer::OnAttach()
	{
		HZ_PROFILE_FUNCTION();

		_unwrapTexture = Texture2D::Create("assets/textures/unwrap_helper.png");

		_framebuffer = Framebuffer::Create({ 1280,720 });
		_activeScene = CreateRef<Scene>();

#if 0
		// Entity
		_squareEntity = _activeScene->CreateEntity("Square");
		_squareEntity.AddComponent<SpriteRendererComponent>(_unwrapTexture);
		_squareEntity.AddComponent<NativeScriptComponent>().Bind<SquareJump>();

		_mainCamera = _activeScene->CreateEntity("Main Camera");
		//_secondaryCamera = _activeScene->CreateEntity("Secondary Camera");
		_mainCamera.AddComponent<CameraComponent>();
		//_secondaryCamera.AddComponent<CameraComponent>().IsPrimary = false;

		_mainCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();
		//_secondaryCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();

#endif // 0

		_sceneHierarchyPanel.SetContext(_activeScene);
	}

	void EditorLayer::OnDetach()
	{
		HZ_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate(Timestep timestep)
	{
		HZ_PROFILE_FUNCTION();
		_updateTimer.Start();

		auto frameBufferSpec = _framebuffer->GetSpecification();
		if (_viewportSize.x > 0.0f && _viewportSize.y > 0.0f &&
			(frameBufferSpec.Width != _viewportSize.x || frameBufferSpec.Height != _viewportSize.y))
		{
			_activeScene->OnViewportResize((uint32_t)_viewportSize.x, (uint32_t)_viewportSize.y);
		}
		_framebuffer->Resize((uint32_t)_viewportSize.x, (uint32_t)_viewportSize.y);

		CalculateFPS(timestep);

#if !HZ_PROFILE
		//SafetyShutdownCheck();
#endif // !HZ_PROFILE

		Renderer2D::ResetStats();
		// Render
		_framebuffer->Bind();
		RenderCommand::SetClearColor(_clearColor);
		RenderCommand::Clear();

		// Update Scene
		_activeScene->OnUpdate(timestep);

		_framebuffer->Unbind();

		_updateTimer.Stop();
	}

	void EditorLayer::OnImGuiRender(Timestep timestep)
	{
		HZ_PROFILE_FUNCTION();

		static bool dockSpaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("DockSpace", &dockSpaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		auto originalWindowMinSize = style.WindowMinSize;
		style.WindowMinSize.x = 370.0f;

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize = originalWindowMinSize;

		DrawFileMenu();
		DrawStats(timestep);
		DrawViewport();
		DrawTools();

		_sceneHierarchyPanel.OnImGuiRender(timestep);

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(OnKeyPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& event)
	{
		if (event.GetRepeatCount() > 0)
		{
			return false;
		}

		bool isImGuizmoInUse = ImGuizmo::IsUsing();
		bool isControlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool isShiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		switch (event.GetKeyCode())
		{
		case Key::N:
			if (isControlPressed)
			{
				NewScene();
			}
			break;
		case Key::O:
			if (isControlPressed)
			{
				OpenScene();
			}
			break;
		case Key::S:
			if (isControlPressed && isShiftPressed)
			{
				SaveSceneAs();
			}
			break;
			// Gizmos 
		case Key::Q:
			if (!isImGuizmoInUse)
			{
				_gizmoType = -1;
			}
			break;
		case Key::W:
			if (!isImGuizmoInUse)
			{
				_gizmoType = ImGuizmo::TRANSLATE;
			}
			break;
		case Key::E:
			if (!isImGuizmoInUse)
			{
				_gizmoType = ImGuizmo::SCALE;
				_gizmoSpace = ImGuizmo::LOCAL;
			}
			break;
		case Key::R:
			if (!isImGuizmoInUse)
			{
				_gizmoType = ImGuizmo::ROTATE;
			}
			break;
		case Key::Z:
			if (!isImGuizmoInUse)
			{
				_gizmoSpace = _gizmoSpace == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
			}
			break;
		}


		return true;
	}

	bool EditorLayer::NewScene()
	{
		if (FileDialogs::NewFile())
		{
			_activeScene = CreateRef<Scene>();
			_activeScene->OnViewportResize((uint32_t)_viewportSize.x, (uint32_t)_viewportSize.y);
			_sceneHierarchyPanel.SetContext(_activeScene);
			return true;
		}

		return false;
	}

	void EditorLayer::OpenScene()
	{
		auto filePath = FileDialogs::OpenFile("Hazel Scene (*.hazel)\0*.hazel\0");
		if (!filePath.empty())
		{
			if (!NewScene())
			{
				return;
			}

			SceneSerializer serializer(_activeScene);
			serializer.Deserialize(filePath);
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		auto filePath = FileDialogs::SaveFile("Hazel Scene (*.hazel)\0*.hazel\0");

		if (!filePath.empty())
		{
			SceneSerializer serializer(_activeScene);
			serializer.Serialize(filePath);
		}
	}

	void EditorLayer::DrawFileMenu()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
				{
					NewScene();
				}

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
				{
					OpenScene();
				}

				//if (ImGui::MenuItem("Save", "Ctrl+S"))
				//{
				//	SceneSerializer serializer(_activeScene);
				//	serializer.Serialize("assets/scenes/Example.hazel");
				//}

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
					SaveSceneAs();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
				{
					HZ_LCRITICAL("Exiting application.");
					Application::Get().Stop();
				}
				ImGui::EndMenu();
			}
			ImGui::Text("\tFPS : %i", _currentFPS);
			ImGui::EndMenuBar();
		}
	}

	void EditorLayer::DrawViewport()
	{
		HZ_PROFILE_FUNCTION();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });

		ImGui::Begin("Viewport");

		_isViewportFocused = ImGui::IsWindowFocused();
		_isViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!_isViewportFocused && !_isViewportHovered);

		auto viewportPanelSize = ImGui::GetContentRegionAvail();
		_viewportSize = { viewportPanelSize.x,viewportPanelSize.y };

		auto textureID = _framebuffer->GetColorAttachmentRenderID();
		ImGui::Image((void*)((uint64_t)textureID), { _viewportSize.x,_viewportSize.y }, ImVec2{ 0,1 }, ImVec2{ 1,0 });

		// Gizmos


		if (auto selectedEntity = _sceneHierarchyPanel.GetSelectedEntity();
			selectedEntity != Entity::Null && _gizmoType != -1)
		{
			if (auto cameraEntity = _activeScene->GetPrimaryCameraEntity();
				cameraEntity != Entity::Null)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();

				auto windowWidth = (float)ImGui::GetWindowWidth();
				auto windowHeight = (float)ImGui::GetWindowHeight();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
				
				const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
				const auto& cameraProjection = camera.GetProjection();
				glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

				// Entity transform
				auto& transformComponent = selectedEntity.GetComponent<TransformComponent>();
				auto transform = transformComponent.GetTransform();

				// Snapping
				bool snap = Input::IsKeyPressed(Key::LeftControl);
				float snapValue = 0.5f; // Snap to 0.5m for position and scale.

				// Snap to 45 degrees for rotation.
				if (_gizmoType == ImGuizmo::ROTATE)
				{
					snapValue = 45.0f;
				}

				float snapValues[3] = { snapValue,snapValue,snapValue };

				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
					(ImGuizmo::OPERATION)_gizmoType, (ImGuizmo::MODE)_gizmoSpace, glm::value_ptr(transform),
					nullptr, snap ? snapValues : nullptr);

				if (ImGuizmo::IsUsing())
				{
					glm::vec3 position, rotation, scale;
					if (Math::DecomposeTransform(transform, position, rotation, scale))
					{
						transformComponent.Position = position;
						auto rotationDelta = rotation - transformComponent.Rotation;
						transformComponent.Rotation += rotationDelta;
						transformComponent.Scale = scale;
					}
				}
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void EditorLayer::DrawStats(Timestep timestep)
	{
		auto stats = Renderer2D::GetStats();

		ImGui::Begin("Stats");
		ImGui::Text("Renderer 2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quad Count: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::Separator();
		auto cycle = (glm::sin(Platform::GetTime()) + 1.0f) * 0.5f;
		ImGui::Text("Ms per frame: %d", _updateTimer.GetProfileResult().ElapsedTime.count() / 1000);

		ImGui::End();
	}

	void EditorLayer::DrawTools()
	{
		ImGui::Begin("Tools");

		if (ImGui::Button("Show Demo Window"))
		{
			_isDemoWidowOpen = !_isDemoWidowOpen;
		}

		if (_isDemoWidowOpen)
		{
			ImGui::ShowDemoWindow(&_isDemoWidowOpen);
		}
		ImGui::Separator();

		//Since we can remove and add component need to disable this for now, to revamp later.
		/*auto& mainCamera = _mainCamera.GetComponent<CameraComponent>();
		auto& secondaryCamera = _secondaryCamera.GetComponent<CameraComponent>();

		if (ImGui::Checkbox("Secondary Camera", &_isOnSecondCamera))
		{
			mainCamera.IsPrimary = !_isOnSecondCamera;
			secondaryCamera.IsPrimary = _isOnSecondCamera;
		}*/
		ImGui::Separator();

		if (ImGui::Button("Create 50 squares"))
		{
			for (int i = 50 - 1; i >= 0; i--)
			{
				auto color = Color::Random();
				auto& newEntity = _activeScene->CreateEntity("Square " + std::to_string(i) + ":" + color.GetHexValue());
				newEntity.AddComponent<SpriteRendererComponent>(color);
				newEntity.GetComponent<TransformComponent>().Position = Random::Vec3();
			}
		}

		ImGui::End();
	}

	/// <summary>
	/// If FPS goes below 2, shutdown.
	/// </summary>
	void EditorLayer::SafetyShutdownCheck()
	{
		// Safety shutdown 
		if (_currentFPS < 2)
		{
			if (_lowFrames == 0)
			{
				HZ_LERROR("Application will shutdown after 10 frames below 2 FPS.", _lowFrames);
			}
			_lowFrames++;
			HZ_LERROR("Frame #{0}", _lowFrames);
			if (_lowFrames >= 10)
			{
				HZ_LCRITICAL("Shuting down.");
				Application::Get().Stop();
			}
		}
		else if (_lowFrames != 0)
		{
			HZ_LINFO("Back to 2 FPS or above.");
			_lowFrames = 0;
		}
	}

	void EditorLayer::CalculateFPS(Timestep timestep)
	{
		HZ_PROFILE_FUNCTION();
		_oneSecondCountDown -= timestep;
		_frameCount++;
		if (_oneSecondCountDown <= 0.0f)
		{
			_currentFPS = _frameCount;
			_oneSecondCountDown = 1.0f;
			_frameCount = 0;
		}
	}
}
