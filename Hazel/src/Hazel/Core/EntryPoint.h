#pragma once
#include "Hazel/Core/Application.h"

extern Hazel::Application* Hazel::CreateApplication(ApplicationCommandLineArgs args);

#ifdef HZ_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
	HZ_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Startup.json");
	Hazel::Log::Init();

	// Create the application using the define function by the client.
	auto* app = Hazel::CreateApplication({ argc, argv });
	HZ_PROFILE_END_SESSION();

	HZ_PROFILE_BEGIN_SESSION("Runtime", "HazelProfile-Runtime.json");
	// application run loop.
	app->Run();
	HZ_PROFILE_END_SESSION();

	HZ_PROFILE_BEGIN_SESSION("Shutdown", "HazelProfile-Shutdown.json");
	// Shutdown
	delete app;
	HZ_PROFILE_END_SESSION();
}

#endif // HZ_PLATFORM_WINDOWS
