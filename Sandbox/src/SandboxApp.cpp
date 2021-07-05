#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "ExampleLayer/ExampleLayer.h"
#include "Sandbox2D.h"
#include "FlappyGame/GameLayer.h"

class Sandbox final : public Hazel::Application
{
public:
	Sandbox()
	{
		HZ_PROFILE_FUNCTION();
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
		// FlappyGame
		//PushLayer(new GameLayer());
	}

	~Sandbox() = default;
};


Hazel::Application* Hazel::CreateApplication(ApplicationCommandLineArgs args)
{
	HZ_PROFILE_FUNCTION();

	return new Sandbox();
}
