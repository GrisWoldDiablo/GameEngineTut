#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Hazel
{
	class Hazelnut final : public Application
	{
	public:
		Hazelnut()
			:Application("Hazel Editor")
		{
			HZ_PROFILE_FUNCTION();

			PushLayer(new EditorLayer());
		}

		~Hazelnut() = default;
	};

	Application* CreateApplication()
	{
		HZ_PROFILE_FUNCTION();

		return new Hazelnut();
	}
}
