#pragma once

#ifdef HZ_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
	Hazel::Log::Init();


	HZ_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Startup.json");
	// Create the application using the define function by the client.
	auto* app = Hazel::CreateApplication();
	HZ_PROFILE_END_SESSION();

	HZ_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Runtime.json");
	// application run loop.
	app->Run();
	HZ_PROFILE_END_SESSION();

	HZ_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Shutdown.json");
	// Shutdown
	delete app;
	HZ_PROFILE_END_SESSION();
}

#endif // HZ_PLATFORM_WINDOWS
