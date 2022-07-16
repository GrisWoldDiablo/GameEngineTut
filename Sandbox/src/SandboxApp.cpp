#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "ExampleLayer/ExampleLayer.h"
#include "Sandbox2D.h"
#include "FlappyGame/GameLayer.h"

class Sandbox final : public Hazel::Application
{
public:
	Sandbox(Hazel::ApplicationSpecification& specification)
		:Application(specification)
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

	ApplicationSpecification spec;
	spec.Name = "Sandbox";
	spec.WorkingDirectory = "../Hazelnut";
	spec.CommandLineArgs = args;

	return new Sandbox(spec);
}
