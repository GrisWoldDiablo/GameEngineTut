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

	_checkerboardTexture = Hazel::Texture2D::Create("assets/textures/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{
	HZ_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Hazel::Timestep timestep)
{
	HZ_PROFILE_FUNCTION();

	_cameraController.OnUpdate(timestep);

	CalculateFPS(timestep);


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

	{
		HZ_PROFILE_SCOPE("Renderer Prep");
		// Render
		Hazel::RenderCommand::EnableDepthTest();
		Hazel::RenderCommand::SetClearColor(_clearColor);
		Hazel::RenderCommand::Clear();
	}

	{
		HZ_PROFILE_SCOPE("Renderer Draw");
		Hazel::Renderer2D::BeginScene(_cameraController.GetCamera());

		// Background to be drawn first behind everything
		Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, _checkerboardTexture, glm::vec2(10.0f), { 0.9f, 0.9f, 0.8f, 1.0f });

		Hazel::RenderCommand::ReadOnlyDepthTest();

		if (_shouldCreateSquares)
		{
			HZ_PROFILE_SNAPSHOT("_shouldCreateSquares");
			if (!_isCreatingSquares)
			{
				HZ_PROFILE_SNAPSHOT("_isCreatingSquares");
				_isCreatingSquares = true;
				auto thread = std::thread([this] { CreateSquares(); });
				thread.detach();
			}
			_shouldCreateSquares = false;
		}
		auto size = _squares.size();
		for (int i = 0; i < size; i++)
		{
			Hazel::Renderer2D::DrawQuad(_squares[i]->Position, _squares[i]->Size, _squares[i]->Color);
		}

		Hazel::Renderer2D::EndScene();
	}
	UpdateSquareList();
}

void Sandbox2D::CreateSquares()
{
	HZ_PROFILE_FUNCTION();

	// Multithreading creation of squares.
	_squareCreationThreads.clear();

	int newqty = _amountOfSquares - _squares.size();
	static const int divider = 100; // How many squares each thread can create.
	int amountOfThreads = newqty / divider;
	for (int i = 0; i < amountOfThreads; i++)
	{
		_squareCreationThreads.push_back(std::thread([this]() { CreateSquare(divider); }));
	}

	int remainder = newqty % divider; // Create the remainder squares.
	_squareCreationThreads.push_back(std::thread([this, remainder] { CreateSquare(remainder); }));

	for (auto& thread : _squareCreationThreads)
	{
		thread.join();
	}

	SortSquares();

	_isCreatingSquares = false;
}

void Sandbox2D::CreateSquare(int amount)
{
	HZ_PROFILE_FUNCTION();


	// If lock is available create the square and push it onto the vector.
	// If not block the thread.

	for (int i = 0; i < amount; i++)
	{
		HZ_PROFILE_SCOPE("CreateSquare");
		Hazel::Ref<Square> square = Hazel::CreateRef<Square>(Square
			{
				Hazel::Random::Vec3() * Hazel::Random::Range(-2.0f,2.0f),
				Hazel::Random::Vec2() * Hazel::Random::Range(1.5f, 5.0f),
				Hazel::Random::Vec4(),
			});
		square->Position.z = glm::clamp(square->Position.z, 0.1f, 0.9f);
		{
			std::lock_guard lock(_mutex);
			_squares.push_back(square);
		}
	}
}

void Sandbox2D::OnImGuiRender(Hazel::Timestep timestep)
{
	HZ_PROFILE_FUNCTION();
	DrawMainGui();
	DrawSquaresGui();
}

void Sandbox2D::UpdateSquareList()
{
	HZ_PROFILE_FUNCTION();
	if (_addSquare)
	{
		_amountOfSquares++;
		_shouldCreateSquares = true;
	}

	if (_clearSquares)
	{
		_squares.clear();
		_amountOfSquares = 0;
	}
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

	ImGui::ColorEdit4("Back Color", glm::value_ptr(_clearColor));
	auto amount = _squares.size();
	ImGui::Text("Squares Quantity: %d / %d", amount, _amountOfSquares);
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
		if (ImGui::Button("OK") || Hazel::Input::IsKeyPressed(HZ_KEY_ENTER))
		{
			if (_amountToAdd > 0)
			{
				_amountOfSquares += _amountToAdd;
				_shouldCreateSquares = true;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}

void Sandbox2D::DrawSquaresGui()
{
	HZ_PROFILE_FUNCTION();
	ImGui::Begin("Squares", nullptr);

	int indexToRemove = -1;
	int vectorSize = glm::clamp((int)_squares.size(), 0, 100);
	for (int i = 0; i < vectorSize; i++)
	{
		ImGui::PushID(i + _amountOfSquares);
		ImGui::Text("Square #%d", i);
		ImGui::SameLine();
		if (ImGui::Button("Remove"))
		{
			indexToRemove = i;
		}
		ImGui::SameLine();
		if (ImGui::Button("Randomize"))
		{
			*_squares[i] = {
				Hazel::Random::RangeVec3({ -2.0f,2.0f },{ -2.0f,2.0f },{ 0.0f,0.9f }),
				Hazel::Random::RangeVec2({ 0.5f,3.0f },{ 0.5f,3.0f }),
				Hazel::Random::RangeVec4({ 0.5f,1.0f },{ 0.5f,1.0f },{ 0.5f,1.0f },{ 0.5f,1.0f }),
			};
		}
		ImGui::PopID();

		ImGui::PushID(i + _amountOfSquares * 2);
		if (ImGui::Button("Rand"))
		{
			_squares[i]->Position = Hazel::Random::RangeVec3({ -2.0f,2.0f }, { -2.0f,2.0f }, { 0.0f,0.9f });
		}
		ImGui::SameLine();
		ImGui::SliderFloat2("Position XY", glm::value_ptr(_squares[i]->Position), -5.0f, 5.0f);
		ImGui::SliderFloat("Position Z", &_squares[i]->Position.z, 0.0f, 0.9f);
		ImGui::PopID();

		ImGui::PushID(i + _amountOfSquares * 3);
		if (ImGui::Button("Rand"))
		{
			_squares[i]->Size = Hazel::Random::RangeVec2({ 0.5f,3.0f }, { 0.5f,3.0f });

		}
		ImGui::SameLine();
		ImGui::SliderFloat2("Size", glm::value_ptr(_squares[i]->Size), 0.1f, 5.0f);
		ImGui::PopID();

		ImGui::PushID(i + _amountOfSquares * 4);
		if (ImGui::Button("Rand"))
		{
			_squares[i]->Color = Hazel::Random::RangeVec4({ 0.5f,1.0f }, { 0.5f,1.0f }, { 0.5f,1.0f }, { 0.5f,1.0f });

		}
		ImGui::SameLine();
		ImGui::ColorEdit4("Color", glm::value_ptr(_squares[i]->Color));
		ImGui::PopID();
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
