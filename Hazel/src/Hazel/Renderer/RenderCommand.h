#pragma once
#include "RendererAPI.h"

namespace Hazel
{
	class HAZEL_API RenderCommand
	{
	public:
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
	public:

		inline static void Init()
		{
			_sRendererAPI->Init();
		}
	};
}
