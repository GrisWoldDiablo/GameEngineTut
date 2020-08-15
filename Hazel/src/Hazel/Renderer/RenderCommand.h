#pragma once
#include "RendererAPI.h"

namespace Hazel
{
	class RenderCommand
	{
	public:
		inline static void Init()
		{
			_sRendererAPI->Init();
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			_sRendererAPI->SetViewport(x, y, width, height);
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray)
		{
			_sRendererAPI->DrawIndexed(vertexArray);
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			_sRendererAPI->SetClearColor(color);
		}
		
		inline static void Clear()
		{
			_sRendererAPI->Clear();
		}
		
	private:
		static Ref<RendererAPI> _sRendererAPI;
	};
}
