#include "EditorLayer.h"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>

namespace Hazel
{
	EditorLayer::EditorLayer()
		: Layer("Hazel Editor"), _cameraController(1280.0f / 720.0f, true)
	{
	}

	void EditorLayer::OnAttach()
	{
		HZ_PROFILE_FUNCTION();

		_unwrapTexture = Texture2D::Create("assets/textures/unwrap_helper.png");

		_framebuffer = Framebuffer::Create({ 1280,720 });
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
			_cameraController.Resize(_viewportSize.x, _viewportSize.y);
		}
		_framebuffer->Resize((uint32_t)_viewportSize.x, (uint32_t)_viewportSize.y);

		if (_isViewportFocused)
		{
			_cameraController.OnUpdate(timestep);
		}

		CalculateFPS(timestep);

#if !HZ_PROFILE
		SafetyShutdownCheck();
#endif // !HZ_PROFILE

		Renderer2D::ResetStats();

		{
			HZ_PROFILE_SCOPE("Renderer Prep");
			_framebuffer->Bind();
			// Render
			RenderCommand::SetClearColor(_clearColor);
			RenderCommand::Clear();
		}

		Renderer2D::BeginScene(_cameraController.GetCamera());

		Renderer2D::DrawQuad({ 0.0f,0.0f }, { 5.0f,5.0f }, _unwrapTexture);

		Renderer2D::EndScene();

		if (Input::IsMouseButtonPressed(MouseCode::ButtonLeft))
		{
			HZ_LTRACE("Left Button {0},{1}", Input::GetMouseX(), Input::GetMouseY());
		}
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
		ImGui::ShowDemoWindow(nullptr);

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

		ImGui::Begin("DockSpace Demo", &dockSpaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

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
		DrawConsole();

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
		auto cycle = (glm::sin(Platform::GetTime()) + 1.0f) * 0.5f;
		ImGui::Text("Ms per frame: %d", _updateTimer.GetProfileResult().ElapsedTime.count() / 1000);

		ImGui::End();
	}

	void EditorLayer::DrawConsole()
	{
		ImGui::Begin("Console");

		auto content = ImGui::GetContentRegionAvail();
		ImGui::BeginChild("Core", ImVec2(content.x, content.y / 2.0f));
		{
			ImGui::TextUnformatted(Log::GetOSSCore()._Unchecked_begin(), Log::GetOSSCore()._Unchecked_end());
			ImGui::SetScrollHereY(1.0f);
			ImGui::EndChild();
		}

		ImGui::Separator();

		ImGui::BeginChild("Client");
		{

			ImGui::TextUnformatted(Log::GetOSSClient()._Unchecked_begin(), Log::GetOSSClient()._Unchecked_end());
			ImGui::SetScrollHereY(1.0f);
			ImGui::EndChild();
		}

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		// #HACK to remove. 
		if (event.GetEventType() != EventType::WindowResize)
		{
			_cameraController.OnEvent(event);
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
