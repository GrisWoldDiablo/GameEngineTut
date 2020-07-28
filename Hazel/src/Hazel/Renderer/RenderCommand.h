#pragma once
#include "RendererAPI.h"

namespace Hazel
{
	class RenderCommand
	{
	public:
		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
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
		static RendererAPI* _sRendererAPI;
	};

}
