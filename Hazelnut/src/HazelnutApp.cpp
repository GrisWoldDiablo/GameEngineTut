#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Hazel
{
	class Hazelnut final : public Application
	{
	public:
		Hazelnut(ApplicationCommandLineArgs args)
			:Application("Hazel Editor", args)
		{
			HZ_PROFILE_FUNCTION();

			PushLayer(new EditorLayer());
		}

		~Hazelnut() = default;
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		HZ_PROFILE_FUNCTION();

		return new Hazelnut(args);
	}
}
