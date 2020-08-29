#include "Sandbox2D.h"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include "Hazel/Core/Random.h"

template<typename Fn>
class Timer
{
public:
	Timer(const char* name, Fn&& func)
		:_name(name), _func(func), _stopped(false)
	{
		_startTimepoint = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		if (!_stopped)
		{
			Stop();
		}
	}

	void Stop()
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(_startTimepoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

		_stopped = true;

		float duration = (end - start) * 0.001f;
		_func({ _name, duration });
	}

private:
	const char* _name;
	Fn _func;
	bool _stopped;
	std::chrono::time_point<std::chrono::steady_clock> _startTimepoint;
};

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) { _profileResults.push_back(profileResult); })

Sandbox2D::Sandbox2D()
	: Layer("Sandbox 2D"), _cameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
	_checkerboardTexture = Hazel::Texture2D::Create("assets/textures/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(Hazel::Timestep timestep)
{
	PROFILE_SCOPE("Sandbox2D::OnUpdate");

	{
		PROFILE_SCOPE("_cameraController.OnUpdate");
		// Update
		_cameraController.OnUpdate(timestep);
	}

	{
		PROFILE_SCOPE("calculate FPS");
		CalculateFPS(timestep);
	}

	// Safety shutdown 
	if (_currentFPS <= 2)
	{
		HZ_LCRITICAL("Shuting down, FPS went at 2 or bellow");
		Hazel::Application::Get().Stop();
	}

	{
		PROFILE_SCOPE("Renderer Prep");
		// Render
		Hazel::RenderCommand::EnableDepthTest();
		Hazel::RenderCommand::SetClearColor(_clearColor);
		Hazel::RenderCommand::Clear();
	}

	{
		PROFILE_SCOPE("Renderer Draw");
		Hazel::Renderer2D::BeginScene(_cameraController.GetCamera());

		// Background to be drawn first behind everything
		Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, _checkerboardTexture, glm::vec2(10.0f), { 0.9f, 0.9f, 0.8f, 1.0f });

		Hazel::RenderCommand::ReadOnlyDepthTest();
		for (int i = 0; i < _amountOfSquares; i++)
		{
			while (_squares.size() != _amountOfSquares)
			{
				Hazel::Ref<Square> square = Hazel::CreateRef<Square>(Square
					{
						Hazel::Random::RangeVec3({ -2.0f ,2.0f }, { -2.0f ,2.0f }, { 0.0f, 0.9f }),
						Hazel::Random::RangeVec2({ 0.5f, 3.0f }, { 0.5f, 3.0f }),
						Hazel::Random::RangeVec4({ 0.5f, 1.0f }, { 0.5f, 1.0f }, { 0.5f, 1.0f }, { 0.5f, 1.0f }),
					});
				_squares.push_back(square);
				if (_squares.size() == _amountOfSquares)
				{
					SortSquares();
				}
			}
			Hazel::Renderer2D::DrawQuad(_squares[i]->Position, _squares[i]->Size, _squares[i]->Color);
		}

		Hazel::Renderer2D::EndScene();
	}
	UpdateSquareList();
}

void Sandbox2D::OnImGuiRender(Hazel::Timestep timestep)
{
	DrawMainGui();
	DrawSquaresGui();

	ImGui::Begin("Profiling");
	for (auto& result : _profileResults)
	{
		char label[50];
		strcpy(label, "%.3fms : ");
		strcat(label, result.Name);
		ImGui::Text(label, result.Time);
	}
	_profileResults.clear();
	ImGui::End();
}

void Sandbox2D::UpdateSquareList()
{
	if (_addSquare)
	{
		_amountOfSquares++;
	}

	if (_clearSquares)
	{
		_squares.clear();
		_amountOfSquares = 0;
	}
}

void Sandbox2D::DrawMainGui()
{
	PROFILE_SCOPE("Draw ImGUI");
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

	ImGui::ColorEdit4("Back Color", glm::value_ptr(_clearColor));

	ImGui::Text("Squares Quantity: %d", _amountOfSquares);
	ImGui::SameLine();
	_addSquare = ImGui::Button("Add");
	ImGui::SameLine();
	_clearSquares = ImGui::Button("Clear");
	ImGui::SameLine();
	if (ImGui::Button("Add Amount"))
	{
		ImGui::OpenPopup("Add_Amount");
	}
	ImGui::SameLine();
	if (ImGui::Button("Sort"))
	{
		SortSquares();
	}

	if (ImGui::BeginPopup("Add_Amount"))
	{
		ImGui::InputInt("Amount", &_amountToAdd);
		if (ImGui::Button("OK"))
		{
			if (_amountToAdd > 0)
			{
				_amountOfSquares += _amountToAdd;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}

void Sandbox2D::DrawSquaresGui()
{
	PROFILE_SCOPE("Draw SquaresGUI");
	ImGui::Begin("Squares", nullptr);

	int indexToRemove = -1;
	int index = 0;

	for (auto& square : _squares)
	{
		ImGui::PushID(index + _amountOfSquares);
		ImGui::Text("Square #%d", index);
		ImGui::SameLine();
		if (ImGui::Button("Remove"))
		{
			indexToRemove = index;
		}
		ImGui::SameLine();
		if (ImGui::Button("Randomize"))
		{
			*square = {
				Hazel::Random::RangeVec3({ -2.0f,2.0f },{ -2.0f,2.0f },{ 0.0f,0.9f }),
				Hazel::Random::RangeVec2({ 0.5f,3.0f },{ 0.5f,3.0f }),
				Hazel::Random::RangeVec4({ 0.5f,1.0f },{ 0.5f,1.0f },{ 0.5f,1.0f },{ 0.5f,1.0f }),
			};
		}
		ImGui::PopID();

		ImGui::PushID(index + _amountOfSquares * 2);
		if (ImGui::Button("Rand"))
		{
			square->Position = Hazel::Random::RangeVec3({ -2.0f,2.0f }, { -2.0f,2.0f }, { 0.0f,0.9f });
		}
		ImGui::SameLine();
		ImGui::SliderFloat2("Position XY", glm::value_ptr(square->Position), -5.0f, 5.0f);
		ImGui::SliderFloat("Position Z", &square->Position.z, 0.0f, 0.9f);
		ImGui::PopID();

		ImGui::PushID(index + _amountOfSquares * 3);
		if (ImGui::Button("Rand"))
		{
			square->Size = Hazel::Random::RangeVec2({ 0.5f,3.0f }, { 0.5f,3.0f });

		}
		ImGui::SameLine();
		ImGui::SliderFloat2("Size", glm::value_ptr(square->Size), 0.1f, 5.0f);
		ImGui::PopID();

		ImGui::PushID(index + _amountOfSquares * 4);
		if (ImGui::Button("Rand"))
		{
			square->Color = Hazel::Random::RangeVec4({ 0.5f,1.0f }, { 0.5f,1.0f }, { 0.5f,1.0f }, { 0.5f,1.0f });

		}
		ImGui::SameLine();
		ImGui::ColorEdit4("Color", glm::value_ptr(square->Color));
		ImGui::PopID();

		index++;
	}

	if (indexToRemove != -1)
	{
		_squares.erase(_squares.begin() + indexToRemove);
		_amountOfSquares--;
	}

	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event)
{
	_cameraController.OnEvent(event);
}

void Sandbox2D::CalculateFPS(Hazel::Timestep timestep)
{
	_oneSecondCountDown -= timestep;
	_frameCount++;
	if (_oneSecondCountDown <= 0.0f)
	{
		_currentFPS = _frameCount;
		_oneSecondCountDown = 1.0f;
		_frameCount = 0;
	}
}
