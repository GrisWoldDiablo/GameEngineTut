#include <Hazel.h>
#include <imgui/imgui.h>

class ExampleLayer final : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
		_color = Hazel::Application::Get().ClearColor;
		_cameraPosition = &Hazel::Application::Get().CameraPosition;
		_cameraRotation = &Hazel::Application::Get().CameraRotation;
	}
	void OnUpdate() override
	{
		auto [xPos, yPos] = Hazel::Input::GetMousePosition();

		if (Hazel::Input::IsKeyPressed(340))
		{
			HZ_LTRACE("{0}, {1}", xPos, yPos);
		}

		if (Hazel::Input::IsKeyPressed(HZ_KEY_TAB))
		{
			HZ_LTRACE("Tab being pressed!!");
		}

		if (Hazel::Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_MIDDLE))
		{
			HZ_LTRACE("Color : R({0}), G({1}), B({2})", _color[0], _color[1], _color[2]);
			std::system("clear");
		}

		if (Hazel::Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_RIGHT))
		{
			HZ_LDEBUG("Random float: {0}, Random Int range: {1}, Random float range: {2}",
				Hazel::Random::Float(), Hazel::Random::Range(-10, 10), Hazel::Random::Range(-10.0f, 10.0f));
			HZ_LDEBUG("Random double: {0}, Random double range: {1}",
				Hazel::Random::Double(), Hazel::Random::Range(-10.0, 10.0));
			HZ_LINFO("Random vec2: {0}, Random vec3: {1}, Random vec4: {2}", Hazel::Random::Vec2(), Hazel::Random::Vec3(), Hazel::Random::Vec4());
		}
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Select your background Color.");
		ImGui::TextColored(ImVec4(_color[0], _color[1], _color[2], _color[3]), "Color");
		ImGui::ColorEdit4("Color", _color, ImGuiColorEditFlags_InputRGB);
		ImGui::End();
	}

	void OnEvent(Hazel::Event& event) override
	{
		Hazel::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Hazel::KeyPressedEvent>(HZ_BIND_EVENT_FN(OnKeypress));
	}

	bool OnKeypress(Hazel::KeyPressedEvent& event)
	{
		if (event.GetEventType() == Hazel::EventType::KeyPressed)
		{
			auto& e = (Hazel::KeyPressedEvent&)event;
			// Insert what to do with event....
			switch (e.GetKeyCode())
			{
			case HZ_KEY_W:
				_cameraPosition->y += 0.1f;
				break;
			case HZ_KEY_S:
				_cameraPosition->y -= 0.1f;
				break;
			case HZ_KEY_A:
				_cameraPosition->x -= 0.1f;
				break;
			case HZ_KEY_D:
				_cameraPosition->x += 0.1f;
				break;
			case HZ_KEY_Q:
				*_cameraRotation += 1.0f;
				break;
			case HZ_KEY_E:
				*_cameraRotation -= 1.0f;
				break;
			}
		}
		return false;
	}

private:
	float* _color;
	glm::vec3* _cameraPosition;
	float* _cameraRotation;
};

class Sandbox final : public Hazel::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox() = default;
};


Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}
