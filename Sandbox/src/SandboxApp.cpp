#include <Hazel.h>
#include <imgui/imgui.h>
class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
		_color = Hazel::Application::Get().ClearColor;
		_scale = &Hazel::Application::Get().ScaleValue;
		_position = Hazel::Application::Get().Position;
	}

	void OnUpdate() override
	{
		auto [xPos, yPos] = Hazel::Input::GetMousePosition();
		auto state = Hazel::Input::IsKeyPressed(340);
		if (state)
		{
			HZ_LTRACE("{0}, {1}", xPos, yPos);
		}
		if (Hazel::Input::IsKeyPressed(HZ_KEY_TAB))
		{
			HZ_LTRACE("Tab being pressed!!");
		}
		state = Hazel::Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_MIDDLE);
		if (state)
		{
			HZ_LTRACE("Color : R({0}), G({1}), B({2})", _color[0], _color[1], _color[2]);
		}
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Select your background Color.");
		ImGui::TextColored(ImVec4(_color[0], _color[1], _color[2],_color[3]),"Color");
		ImGui::ColorPicker4("Color", _color, ImGuiColorEditFlags_InputRGB);
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
			switch (e.GetKeyCode())
			{
			case HZ_KEY_KP_ADD:
				*_scale += 0.1f;
				break;
			case HZ_KEY_KP_SUBTRACT:
				*_scale -= 0.1f;
				break;
			case HZ_KEY_UP:
				_position[1] += 0.1f;
				break;
			case HZ_KEY_DOWN:
				_position[1] -= 0.1f;
				break;
			case HZ_KEY_LEFT:
				_position[0] -= 0.1f;
				break;
			case HZ_KEY_RIGHT:
				_position[0] += 0.1f;
				break;
			default:
				break;
			}
		}
		return false;
	}
	
private:
	float* _color;
	float* _scale;
	float* _position;
};

class Sandbox : public Hazel::Application
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
