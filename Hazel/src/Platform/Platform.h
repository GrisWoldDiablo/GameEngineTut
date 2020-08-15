#pragma once
#include "Hazel/Renderer/RendererAPI.h"

namespace Hazel
{
	class Platform
	{
	public:
		Platform(Ref<RendererAPI> renderAPI)
			:_rendererAPI(renderAPI)
		{
		}

		inline static float GetTime() { return _sInstance->GetTimeImpl(); }

	private:
		inline float GetTimeImpl() { return _rendererAPI->GetTime(); }

	private:
		Ref<RendererAPI> _rendererAPI;

		// Singleton related 
		static Scope<Platform> _sInstance;
	};
}
