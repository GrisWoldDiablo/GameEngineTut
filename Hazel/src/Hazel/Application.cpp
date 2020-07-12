#include "Application.h"

#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Log.h"

namespace Hazel
{
	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		std::printf("\n");
		HZ_CORE_LDEBUG("Application::Run()");

		WindowResizeEvent e(1280, 720);
		if (e.IsInCategory(EventCategoryApplication))
		{
			// Will run since it is the right category.
			HZ_CORE_LTRACE(e);
		}
		if (e.IsInCategory(EventCategoryMouse))
		{
			// Won't run since it is the wrong category.
			HZ_CORE_LTRACE(e);
		}

		while (true);
	}
}
