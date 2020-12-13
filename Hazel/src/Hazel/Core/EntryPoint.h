#pragma once

#ifdef HZ_PLATFORM_WINDOWS

//int main(int argc, char** argv)
int WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd )
{
	HZ_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Startup.json");
	Hazel::Log::Init();

	// Create the application using the define function by the client.
	auto* app = Hazel::CreateApplication();
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
