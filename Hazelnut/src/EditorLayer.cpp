#include "EditorLayer.h"
#include "NativeScripts.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

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

		// Entity
		_activeScene = CreateRef<Scene>();
		_squareEntity = _activeScene->CreateEntity("Square");
		_squareEntity.AddComponent<SpriteRendererComponent>(_unwrapTexture);
		_squareEntity.AddComponent<NativeScriptComponent>().Bind<SquareJump>();

		_mainCamera = _activeScene->CreateEntity("Main Camera");
		//_secondaryCamera = _activeScene->CreateEntity("Secondary Camera");
		_mainCamera.AddComponent<CameraComponent>();
		//_secondaryCamera.AddComponent<CameraComponent>().IsPrimary = false;

		_mainCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();
		//_secondaryCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();
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

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
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

		DrawStats(timestep);
		DrawViewport();
		DrawTools();

		_sceneHierarchyPanel.OnImGuiRender(timestep);

		ImGui::End();
	}

	void EditorLayer::DrawViewport()
	{
		HZ_PROFILE_FUNCTION();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });

		ImGui::Begin("Viewport");

		_isViewportFocused = ImGui::IsWindowFocused();
		_isViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!_isViewportFocused || !_isViewportHovered);

		auto viewportPanelSize = ImGui::GetContentRegionAvail();
		_viewportSize = { viewportPanelSize.x,viewportPanelSize.y };

		auto textureID = _framebuffer->GetColorAttachmentRenderID();
		ImGui::Image((void*)textureID, { _viewportSize.x,_viewportSize.y }, ImVec2{ 0,1 }, ImVec2{ 1,0 });

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

	void EditorLayer::OnEvent(Event& event)
	{

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
