#pragma once
#include "Core.h"
#include "Log.h"

namespace Hazel
{
	/// <summary>
	/// The application class with the main run loop.
	/// </summary>
	class HAZEL_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

	};

	// To be define by the client.
	Application* CreateApplication();
}
