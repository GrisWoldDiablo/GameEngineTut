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

	_unwrapTexture = Hazel::Texture2D::Create("assets/textures/unwrap_helper.png");
	_checkerboardTexture = Hazel::Texture2D::Create("assets/textures/Checkerboard.png");
	_logoTexture = Hazel::Texture2D::Create("assets/textures/ChernoLogo.png");
	_spriteSheet = Hazel::Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

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

#if 0
	// Cycle Lerp background color
	_lerpValueSin = (glm::sin(Hazel::Platform::GetTime() * _lerpSpeed) + 1.0f) * 0.5f;
	_lerpValueCos = (glm::cos(Hazel::Platform::GetTime() * _lerpSpeed) + 1.0f) * 0.5f;
	_lerpedColor = Hazel::Color::LerpUnclamped(_clearColorA, _clearColorB, _lerpValueSin);

#endif // 0

	Hazel::Renderer2D::ResetStats();
	{
		HZ_PROFILE_SCOPE("Renderer Prep");
		// Render
		//Hazel::RenderCommand::SetDepthMaskReadWrite();
		Hazel::RenderCommand::SetClearColor(_clearColorB);
		Hazel::RenderCommand::Clear();
	}

#if 0

	{
		HZ_PROFILE_SCOPE("Renderer Draw");
		Hazel::Renderer2D::BeginScene(_cameraController.GetCamera());

		Hazel::Renderer2D::DrawRotatedQuad({ _cameraController.GetPosition().x, _cameraController.GetPosition().y, -0.2f }, { 10.0f * _cameraController.GetZoomLevel(), 10.0f * _cameraController.GetZoomLevel() }, _cameraController.GetRotation(), _unwrapTexture, glm::vec2(10.0f), _lerpedColor);
		Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 20.0f , 20.0f }, _checkerboardTexture, glm::vec2(10.0f), Hazel::Color(0.9f, 0.9f, 0.8f, 1.0f));
		Hazel::Renderer2D::DrawQuad({ -2.5f, -1.0f, 0.0f }, { 5.0f, 0.5f }, _checkerboardTexture, glm::vec2(5.0f, 0.25f), _lerpedColor);
		Hazel::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f,0.8f }, Hazel::Color::Cyan);
		Hazel::Renderer2D::DrawQuad({ 0.5f, -0.5f, 0.5 }, { 0.5f,0.75f }, Hazel::Color::Red);
		static float rotation = 0.0f;
		rotation += timestep * 50.0f;
		Hazel::Renderer2D::DrawRotatedQuad({ 0.5f, 0.5f, 0.5 }, { 0.5f,0.5f }, rotation, Hazel::Color::Blue);

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

		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				auto color = Hazel::Color((x + 5.0f) / 10.0f, (y + 5.0f) / 10.0f, _lerpValueSin, 0.8f);
				Hazel::Renderer2D::DrawRotatedQuad({ x + _lerpValueSin, y + _lerpValueCos, }, { 0.45f, 0.45f }, Hazel::Platform::GetTime() * 25.0f, color);
			}
		}

		{
			std::lock_guard lock(_mutex);
			for (const auto& square : _squares)
			{
				Hazel::Renderer2D::DrawQuad(square->Position, square->Size, square->Color);
			}
		}

		Hazel::Renderer2D::EndScene();


		Hazel::Renderer2D::BeginScene(_cameraController.GetCamera(), true);

		Hazel::Renderer2D::DrawQuad({ -5.0f, 5.0f, 1.0f }, { 5.0f, 5.0f }, _logoTexture);

		Hazel::Renderer2D::EndScene();



	}
	UpdateSquareList();

#endif // 0

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
	Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, _spriteSheet);
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
		auto position = Hazel::Random::RangeVec3({ -15.0f,15.0f }, { -15.0f,15.0f }, { 0.1f,0.9f });
		auto scale = Hazel::Random::Vec2() * Hazel::Random::Range(1.0f, 15.0f);
		auto color = Hazel::Color::Random();
		tempSquares.emplace_back(Hazel::CreateRef<Square>(Square
			{
				position,
				scale,
				color
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
	DrawParticlesGui();
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
			ImGui::SliderFloat2("Position XY", glm::value_ptr(_squares[i]->Position), -15.0f, 15.0f);
			ImGui::SliderFloat("Position Z", &_squares[i]->Position.z, 0.0f, 0.9f);
			ImGui::PopID();

			ImGui::PushID(i + _amountOfSquares * 3);
			if (ImGui::Button("Rand"))
			{
				_squares[i]->Size = Hazel::Random::RangeVec2({ 0.5f,3.0f }, { 0.5f,3.0f });

			}
			ImGui::SameLine();
			ImGui::SliderFloat2("Size", glm::value_ptr(_squares[i]->Size), 0.1f, 15.0f);
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
