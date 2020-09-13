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
	_logoTexture = Hazel::Texture2D::Create("assets/textures/ChernoLogo.png");
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

#if !HZ_PROFILE
	SafetyShutdownCheck();
#endif // !HZ_PROFILE

	// Cycle Lerp background color
	_lerpValueSin = (glm::sin(Hazel::Platform::GetTime() * _lerpSpeed) + 1.0f) * 0.5f;
	_lerpValueCos = (glm::cos(Hazel::Platform::GetTime() * _lerpSpeed) + 1.0f) * 0.5f;
	_lerpedColor = Hazel::Color::LerpUnclamped(_clearColorA, _clearColorB, _lerpValueSin);

	Hazel::Renderer2D::ResetStats();
	{
		HZ_PROFILE_SCOPE("Renderer Prep");
		// Render
		Hazel::RenderCommand::SetClearColor(_lerpedColor);
		Hazel::RenderCommand::Clear();
	}

	{
		HZ_PROFILE_SCOPE("Renderer Draw");
		Hazel::Renderer2D::BeginScene(_cameraController.GetCamera());

		Hazel::Renderer2D::DrawRotatedQuad({ -2.0f, 2.0f, 0.5f }, { 1.0f, 1.0f }, 45, _logoTexture);
		Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 20.0f, 20.0f }, _checkerboardTexture, glm::vec2(10.0f), Hazel::Color(0.9f, 0.9f, 0.8f, 1.0f));
		Hazel::Renderer2D::DrawQuad({ -2.5f, -1.0f, 0.0f }, { 5.0f, 0.5f }, _checkerboardTexture, glm::vec2(5.0f, 0.25f), _lerpedColor);

		Hazel::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f,0.8f }, Hazel::Color::Cyan);
		Hazel::Renderer2D::DrawQuad({ 0.5f, -0.5f, 0.5 }, { 0.5f,0.75f }, Hazel::Color::Red);
		static float rotation = 0.0f;
		rotation += timestep * 50.0f;
		Hazel::Renderer2D::DrawRotatedQuad({ 0.5f, 0.5f, 0.5 }, { 0.5f,0.5f }, rotation, Hazel::Color::Blue);

		//Hazel::RenderCommand::ReadOnlyDepthTest();

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

		{
			std::lock_guard lock(_mutex);
			for (const auto& square : _squares)
			{
				Hazel::Renderer2D::DrawQuad(square->Position, square->Size, square->Color);
			}
		}

		Hazel::Renderer2D::EndScene();

		Hazel::Renderer2D::BeginScene(_cameraController.GetCamera());

		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				auto color = Hazel::Color((x + 5.0f) / 10.0f, (y + 5.0f) / 10.0f, _lerpValueSin, 0.8f);
				Hazel::Renderer2D::DrawRotatedQuad({ x + _lerpValueSin, y + _lerpValueCos, }, { 0.45f, 0.45f }, Hazel::Platform::GetTime() * 25.0f , color);
			}
		}
		Hazel::Renderer2D::EndScene();
	}
	UpdateSquareList();
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

void Sandbox2D::CreateSquares()
{
	HZ_PROFILE_FUNCTION();

	// Multi-threading creation of squares.
	_squareCreationThreads.clear();

	auto newqty = (int)(_amountOfSquares - _squares.size());
	static const int divider = 100; // How many squares each thread can create.
	int amountOfThreads = newqty / divider;

	for (int i = 0; i < amountOfThreads; i++)
	{
		_squareCreationThreads.emplace_back([this]() { CreateSquare(divider); });
	}

	int remainder = newqty % divider; // Create the remainder squares.
	_squareCreationThreads.emplace_back([this, remainder] { CreateSquare(remainder); });

	for (auto& thread : _squareCreationThreads)
	{
		// This will make this current thread wait for all other thread to complete before continuing.
		thread.join();
	}

	// Have to lock before sorting the squares.
	std::lock_guard lock(_mutex);
	SortSquares();

	_isCreatingSquares = false;
}

void Sandbox2D::CreateSquare(int amount)
{
	HZ_PROFILE_FUNCTION();
	std::vector<Hazel::Ref<Square>> tempSquares;
	tempSquares.reserve(amount);
	for (int i = 0; i < amount; i++)
	{
		HZ_PROFILE_SCOPE("CreateSquare");
		tempSquares.emplace_back(Hazel::CreateRef<Square>(Square
			{
				Hazel::Random::RangeVec3({ -5.0f,5.0f }, { -5.0f,5.0f }, { 0.1f, 0.9f }),
				Hazel::Random::Vec2() * Hazel::Random::Range(1.5f, 5.0f),
				Hazel::Color::Random()
			}));
	}

	// Block the thread until lock is available, then inserts the squares onto the vector.
	std::lock_guard lock(_mutex);
	_squares.insert(_squares.end(), tempSquares.begin(), tempSquares.end());

}

void Sandbox2D::OnImGuiRender(Hazel::Timestep timestep)
{
	HZ_PROFILE_FUNCTION();
	//ImGui::ShowDemoWindow(nullptr);
	DrawMainGui();
	DrawSquaresGui();
	DrawStats(timestep);
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

	ImGui::ColorEdit4("Back ColorA", _clearColorA.GetValuePtr());
	ImGui::ColorEdit4("Back ColorB", _clearColorB.GetValuePtr());
	ImGui::ColorEdit4("Lerped Back Color", _lerpedColor.GetValuePtr());
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
	{
		std::lock_guard lock(_mutex);
		for (int i = 0; i < glm::clamp((int)_squares.size(), 0, 10); i++)
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
			ImGui::ColorEdit4("Color", _squares[i]->Color.GetValuePtr());
			ImGui::PopID();
		}
	}

	if (indexToRemove != -1)
	{
		_squares.erase(_squares.begin() + indexToRemove);
		_amountOfSquares--;
	}

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
	ImGui::Text("Time: %f", cycle);
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
