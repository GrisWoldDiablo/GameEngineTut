#include "Sandbox2D.h"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox 2D"), _cameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
	HZ_PROFILE_FUNCTION();

	_spriteSheet = Hazel::Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");
	Hazel::FramebufferSpecification fbSpec;
	fbSpec.Width = 1280;
	fbSpec.Height = 720;
	_framebuffer = Hazel::Framebuffer::Create(fbSpec);
}

void Sandbox2D::OnDetach()
{
	HZ_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Hazel::Timestep timestep)
{
	HZ_PROFILE_FUNCTION();
	_updateTimer.Start();

	_cameraController.OnUpdate(timestep);

	CalculateFPS(timestep);

#if !HZ_PROFILE
	SafetyShutdownCheck();
#endif // !HZ_PROFILE

	Hazel::Renderer2D::ResetStats();

	{
		HZ_PROFILE_SCOPE("Renderer Prep");
		_framebuffer->Bind();
		// Render
		//Hazel::RenderCommand::SetDepthMaskReadWrite();
		Hazel::RenderCommand::SetClearColor(_clearColor);
		Hazel::RenderCommand::Clear();
	}

	Hazel::Renderer2D::BeginScene(_cameraController.GetCamera());

	Hazel::Renderer2D::DrawQuad({ 0.0f,0.0f }, { 10.0f,10.0f }, _spriteSheet);


	Hazel::Renderer2D::EndScene();


	_framebuffer->Unbind();
	_updateTimer.Stop();
}

/// <summary>
/// If FPS goes below 2, shutdown.
/// </summary>
void Sandbox2D::SafetyShutdownCheck()
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
			Hazel::Application::Get().Stop();
		}
	}
	else if (_lowFrames != 0)
	{
		HZ_LINFO("Back to 2 FPS or above.");
		_lowFrames = 0;
	}
}

void Sandbox2D::OnImGuiRender(Hazel::Timestep timestep)
{
	HZ_PROFILE_FUNCTION();
	//ImGui::ShowDemoWindow(nullptr);
	
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
				Hazel::Application::Get().Stop();
			}
			ImGui::EndMenu();
		}
		ImGui::Text("\tFPS : %i", _currentFPS);
		ImGui::EndMenuBar();
	}

	
	DrawMainGui();
	DrawStats(timestep);
	//DrawParticlesGui();

	ImGui::End();
}

void Sandbox2D::DrawMainGui()
{
	HZ_PROFILE_FUNCTION();
	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar);

	auto textureID = _framebuffer->GetColorAttachmentRenderID();
	ImGui::Image((void*)textureID, ImVec2{ 1280.0f, 720.0f });

	ImGui::End();
}

void Sandbox2D::DrawStats(Hazel::Timestep timestep)
{
	auto stats = Hazel::Renderer2D::GetStats();

	ImGui::Begin("Stats", nullptr);
	ImGui::Text("Renderer 2D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quad Count: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
	auto cycle = (glm::sin(Hazel::Platform::GetTime()) + 1.0f) * 0.5f;
	ImGui::Text("Ms per frame: %d", _updateTimer.GetProfileResult().ElapsedTime.count() / 1000);

	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event)
{
	_cameraController.OnEvent(event);
}

void Sandbox2D::CalculateFPS(Hazel::Timestep timestep)
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
