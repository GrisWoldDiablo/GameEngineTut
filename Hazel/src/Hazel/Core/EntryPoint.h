#pragma once

#ifdef HZ_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
	Hazel::Log::Init();

	// Create the application using the define function by the client.
	auto* app = Hazel::CreateApplication();
	
	// application run loop.
	app->Run();
	delete app;
}

#endif // HZ_PLATFORM_WINDOWS
