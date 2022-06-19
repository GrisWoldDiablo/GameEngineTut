#pragma once
#include "Hazel/Renderer/RendererAPI.h"

namespace Hazel
{
	class Platform
	{
	public:
		Platform(const Ref<RendererAPI>& renderAPI)
			:_rendererAPI(renderAPI)
		{}

		static float GetTime() { return _sInstance->GetTimeImpl(); }

	private:
		float GetTimeImpl() { return _rendererAPI->GetTime(); }

	private:
		Ref<RendererAPI> _rendererAPI;

		// Singleton related 
		static Scope<Platform> _sInstance;
	};
}
