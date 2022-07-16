#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Hazel
{
	class Hazelnut final : public Application
	{
	public:
		Hazelnut(const ApplicationSpecification& spec)
			:Application(spec)
		{
			HZ_PROFILE_FUNCTION();

			PushLayer(new EditorLayer());
		}

		virtual ~Hazelnut() override = default;
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		HZ_PROFILE_FUNCTION();

		ApplicationSpecification spec;
		spec.Name = "Hazelnut";
		spec.CommandLineArgs = args;

		return new Hazelnut(spec);
	}
}
