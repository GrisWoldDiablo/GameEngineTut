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
	_stairsTexture = Hazel::SubTexture2D::CreateFromCoords(_spriteSheet, { 7,6 }, { 128,128 });
	_barrelTexture = Hazel::SubTexture2D::CreateFromCoords(_spriteSheet, { 8,2 }, { 128,128 });
	_treeTexture = Hazel::SubTexture2D::CreateFromCoords(_spriteSheet, { 2,1 }, { 128,128 }, { 1,2 });

	// Init Particle props
	_particleProps.ColorBegin = Hazel::Color::Random();
	_particleProps.ColorEnd = Hazel::Color::Random();
	_particleProps.SizeBegin = 0.5f;
	_particleProps.SizeVariation = 0.3f;
	_particleProps.SizeEnd = 0.0f;
	_particleProps.LifeTime = 1.0f;
	_particleProps.Velocity = glm::vec2(0.0f);
	_particleProps.VelocityVaritation = { 3.0f, 3.0f };
	_particleProps.Position = glm::vec2(0.0f);
	_particleProps.RotationSpeedVariation = 35.0f;

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
		// Render
		//Hazel::RenderCommand::SetDepthMaskReadWrite();
		Hazel::RenderCommand::SetClearColor(_clearColor);
		Hazel::RenderCommand::Clear();
	}


	if (Hazel::Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_LEFT))
	{
		auto [x, y] = Hazel::Input::GetMousePosition();
		auto width = Hazel::Application::Get().GetWindow().GetWidth();
		auto height = Hazel::Application::Get().GetWindow().GetHeight();

		auto bounds = _cameraController.GetBounds();

		x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
		y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();

		auto rotation = _cameraController.GetRotation();
		auto rad = glm::radians(rotation);
		auto cos = glm::cos(rad);
		auto sin = glm::sin(rad);
		auto xPrime = x * cos - y * sin;
		auto yPrime = x * sin + y * cos;

		auto pos = _cameraController.GetPosition();
		_particleProps.Position = { xPrime + pos.x, yPrime + pos.y };
		
		for (size_t i = 0; i < _particlesAmountPerFrame; i++)
		{
			_particleSystem.Emit(_particleProps);
		}
	}

	//Hazel::RenderCommand::SetDepthMaskReadOnly();
	_particleSystem.OnUpdate(timestep);
	_particleSystem.OnRender(_cameraController.GetCamera());
	
	Hazel::Renderer2D::BeginScene(_cameraController.GetCamera());
	Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, _stairsTexture);
	Hazel::Renderer2D::DrawQuad({ -1.0f, 0.5f, 1.0f }, { 1.0f, 2.0f }, _treeTexture);
	Hazel::Renderer2D::DrawQuad({ 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, _barrelTexture);
	Hazel::Renderer2D::EndScene();

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
	DrawMainGui();
	DrawStats(timestep);
	DrawParticlesGui();
}

void Sandbox2D::DrawMainGui()
{
	HZ_PROFILE_FUNCTION();
	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_MenuBar);

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

	ImGui::ColorEdit4("Back Color", _clearColor.GetValuePtr());

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

void Sandbox2D::DrawParticlesGui()
{
	auto currentPoolSize = _particlesPoolSize;
	ImGui::Begin("Particles", nullptr);
	ImGui::SliderInt("Max Amount Rendered", &_particlesPoolSize, 1, 100000);
	if (_particlesPoolSize != currentPoolSize)
	{
		_particleSystem.SetParticlePoolSize(_particlesPoolSize);
	}
	ImGui::SliderInt("Amount Per Frame", &_particlesAmountPerFrame, 1, 500);
	ImGui::SliderFloat("LifeTime", &_particleProps.LifeTime,0.0f,5.0f);
	ImGui::Text("Colors");
	ImGui::ColorEdit4("Begin", _particleProps.ColorBegin.GetValuePtr());
	ImGui::ColorEdit4("End", _particleProps.ColorEnd.GetValuePtr());
	ImGui::Text("Size");
	ImGui::SliderFloat("Begin", &_particleProps.SizeBegin, 0.0f, 10.0f);
	ImGui::SliderFloat("End", &_particleProps.SizeEnd, 0.0f, 10.0f);
	ImGui::SliderFloat2("Velocity Variation", glm::value_ptr(_particleProps.VelocityVaritation), 0.0f, 10.0f);
	ImGui::SliderFloat("Rotation Speed Variation", &_particleProps.RotationSpeedVariation, 0.0f, 360.0f);
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
