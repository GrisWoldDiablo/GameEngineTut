#include <Hazel.h>
#include <imgui/imgui.h>

class ExampleLayer final : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
		_color = Hazel::Application::Get().ClearColor;
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
				Hazel::Random::GetRandom(), Hazel::Random::GetRandomRange(1, 10), Hazel::Random::GetRandomRange(1.0f, 10.0f));
			HZ_LINFO("Random vec2: {0}, Random vec3: {1}", Hazel::Random::GetRandomVec2(), Hazel::Random::GetRandomVec3());
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
			// Insert what to do with event...
		}
		return false;
	}

private:
	float* _color;
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
