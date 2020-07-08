#pragma once

#ifdef HZ_PLATFORM_WINDOWS

extern Hazel::Application* Hazel::CreateApplication();

int main(int argc, char** argv)
{
	Hazel::Log::Init();
	HZ_CORE_DEBUG("DEBUG Core Log!");
	HZ_CORE_TRACE("TRACE Core Log!");
	HZ_CORE_INFO("INFO Core Log!");
	HZ_CORE_WARN("WARN Core Log!");
	HZ_CORE_ERROR("ERROR Core Log!");
	HZ_CORE_CRITICAL("CRITICAL Core Log!");

	HZ_DEBUG("DEBUG Client Log!");
	HZ_TRACE("TRACE Client Log!");
	HZ_INFO("INFO Client Log!");
	HZ_WARN("WARN Client Log!");
	HZ_ERROR("ERROR Client Log!");
	HZ_CRITICAL("CRITICAL Client Log!");

	// Create the application using the define function by the client.
	auto app = Hazel::CreateApplication();
	
	// application run loop.
	app->Run();
	delete app;
}

#endif // HZ_PLATFORM_WINDOWS
