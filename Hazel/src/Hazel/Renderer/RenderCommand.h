#pragma once
#include "RendererAPI.h"

namespace Hazel
{
	class RenderCommand
	{
	public:
		static void Init()
		{
			HZ_PROFILE_FUNCTION();
			
			_sRendererAPI->Init();
		}

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			HZ_PROFILE_FUNCTION();
			
			_sRendererAPI->SetViewport(x, y, width, height);
		}

		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count = 0)
		{
			HZ_PROFILE_FUNCTION();

			_sRendererAPI->DrawIndexed(vertexArray, count);
		}

		static void SetClearColor(const Color& color)
		{
			HZ_PROFILE_FUNCTION();

			_sRendererAPI->SetClearColor(color);
		}
		
		static void Clear()
		{
			HZ_PROFILE_FUNCTION();

			_sRendererAPI->Clear();
		}


		static void EnableDepthTest()
		{
			HZ_PROFILE_FUNCTION();

			_sRendererAPI->EnableDepthTest();
		}

		static void ReadOnlyDepthTest()
		{
			HZ_PROFILE_FUNCTION();

			_sRendererAPI->SetDepthTestReadOnly();
		}
		
	private:
		static Ref<RendererAPI> _sRendererAPI;
	};
}
