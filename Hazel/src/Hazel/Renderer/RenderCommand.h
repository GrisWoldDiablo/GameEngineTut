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

		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			HZ_PROFILE_FUNCTION();

			_sRendererAPI->DrawIndexed(vertexArray, indexCount);
		}

		static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
		{
			HZ_PROFILE_FUNCTION();

			_sRendererAPI->DrawLines(vertexArray, vertexCount);
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

		static void SetDepthMaskReadWrite()
		{
			HZ_PROFILE_FUNCTION();

			_sRendererAPI->SetDepthMaskReadWrite();
		}

		static void SetDepthMaskReadOnly()
		{
			HZ_PROFILE_FUNCTION();

			_sRendererAPI->SetDepthMaskReadOnly();
		}

		static void SetLineWidth(float width)
		{
			_sRendererAPI->SetLineWidth(width);
		}

	private:
		static Ref<RendererAPI> _sRendererAPI;
	};
}
