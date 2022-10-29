#pragma once
#include "Hazel/Core/Application.h"

extern auto Hazel::CreateApplication(ApplicationCommandLineArgs args) -> Application*;

#ifdef HZ_PLATFORM_WINDOWS

namespace Hazel
{
	int Main(int argc, char** argv)
	{
		HZ_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Startup.json");
		Log::Init();

		// Create the application using the define function by the client.
		auto* app = CreateApplication({argc, argv});
		HZ_PROFILE_END_SESSION();

		HZ_PROFILE_BEGIN_SESSION("Runtime", "HazelProfile-Runtime.json");
		// application run loop.
		app->Run();
		HZ_PROFILE_END_SESSION();

		HZ_PROFILE_BEGIN_SESSION("Shutdown", "HazelProfile-Shutdown.json");
		// Shutdown
		delete app;
		HZ_PROFILE_END_SESSION();

		return 0;
	}
}

#ifdef HZ_DIST

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE HInstPrev, PSTR cmdline, int cmdshow)
{
	return Hazel::Main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return Hazel::Main(argc, argv);
}

#endif // HZ_DIST

#endif // HZ_PLATFORM_WINDOWS
