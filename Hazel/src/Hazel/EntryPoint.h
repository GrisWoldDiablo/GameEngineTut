#pragma once

#ifdef HZ_PLATFORM_WINDOWS

extern Hazel::Application* Hazel::CreateApplication();

int main(int argc, char** argv)
{
	Hazel::Log::Init();
	HZ_CORE_LDEBUG("HZ_CORE_LDEBUG!");
	HZ_CORE_LTRACE("HZ_CORE_LTRACE!");
	HZ_CORE_LINFO("HZ_CORE_LINFO!");
	HZ_CORE_LWARN("HZ_CORE_LWARN!");
	HZ_CORE_LERROR("HZ_CORE_LERROR!");
	HZ_CORE_LCRITICAL("HZ_CORE_LCRITICAL!");

	HZ_LDEBUG("HZ_LDEBUG");
	HZ_LTRACE("HZ_LTRACE");
	HZ_LINFO("HZ_LINFO");
	HZ_LWARN("HZ_LWARN");
	HZ_LERROR("HZ_LERROR");
	HZ_LCRITICAL("HZ_LCRITICAL");

	// Create the application using the define function by the client.
	auto app = Hazel::CreateApplication();
	
	// application run loop.
	app->Run();
	delete app;
}

#endif // HZ_PLATFORM_WINDOWS
