#pragma once

#include "Hazel/Renderer/RendererAPI.h"

namespace Hazel
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void Init() override;

		void EnableDepthTest() override;
		void ReadOnlyDepthTest() override;

		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void SetClearColor(const Color& color) override;
		void Clear() override;
		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count = 0) override;
		float GetTime() override;
	};
}
