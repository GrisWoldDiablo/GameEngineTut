#include <Hazel.h>


class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example") {}

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
			HZ_LTRACE("{0}", (char)e.GetKeyCode());
		}
		return false;
	}
};

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new Hazel::ImGuiLayer());
	}

	~Sandbox()
	{
		
	}
};


Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}
