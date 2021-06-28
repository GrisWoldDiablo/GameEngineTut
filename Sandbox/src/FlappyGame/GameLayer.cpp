#include "GameLayer.h"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>

GameLayer::GameLayer()
	: Layer("Sandbox 2D"), _cameraController(1280.0f / 720.0f, true)
{
	_cameraController.SetZoomLevel(5.0f);
	_cameraController.SetDefaults();
}

void GameLayer::OnAttach()
{
	_level.Init();
}

void GameLayer::OnDetach()
{}

void GameLayer::OnUpdate()
{
	_level.OnUpdate(Hazel::Time::GetTimestep());
	const auto& playerPos = _level.GetPlayer().GetPosition();
	_cameraController.SetPosition({ playerPos.x,playerPos.y,0.0f });
	_cameraController.OnUpdate();

	Hazel::RenderCommand::SetDepthMaskReadWrite();
	Hazel::RenderCommand::SetClearColor({ 0.4f, 0.4f, 0.4f, 1.0f });
	Hazel::RenderCommand::Clear();

	Hazel::Renderer2D::BeginScene(_cameraController.GetCamera());

	Hazel::RenderCommand::SetDepthMaskReadOnly();

	_level.OnRender();

	Hazel::Renderer2D::EndScene();

}

void GameLayer::OnImGuiRender()
{
	ImGui::Begin("Settings");

	_level.OnImGuiRender();

	ImGui::End();


	uint32_t playerScore = _level.GetPlayer().GetScore();
	std::string scoreStr = std::string("Score: ") + std::to_string(playerScore);
	ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), 48.0f, ImGui::GetWindowPos(), 0xff1111ff, scoreStr.c_str());
}

void GameLayer::OnEvent(Hazel::Event& event)
{
	_cameraController.OnEvent(event);
}
