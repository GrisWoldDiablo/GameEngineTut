#include <Hazel.h>

#define BIND_EVENT_FN(x) std::bind(&ExampleLayer::x, this, std::placeholders::_1)

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example") {}

	void OnUpdate() override
	{
		//HZ_LINFO("ExampleLayer::Update");
	}

	void OnEvent(Hazel::Event& event) override
	{
		Hazel::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Hazel::KeyPressedEvent>(BIND_EVENT_FN(OnKeypress));
		//HZ_LTRACE("{0}", event);
	}
	
	bool OnKeypress(Hazel::KeyPressedEvent& event)
	{
		if (event.GetKeyCode() == 81) // 'q'
		{
			HZ_LINFO("Example Layer : {0}", event);
			return true;
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
