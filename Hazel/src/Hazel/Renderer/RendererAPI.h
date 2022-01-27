#pragma once

#include <glm/glm.hpp>

#include "VertexArray.h"
#include "Hazel/Core/Color.h"

namespace Hazel
{
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL, DirectX, Vulkan
		};

	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;

		// #TODO: Temporary, will need to have render queue with sorting.
		virtual void SetDepthMaskReadWrite() = 0;
		virtual void SetDepthMaskReadOnly() = 0;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const Color& color) = 0;

		virtual void Clear() = 0;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

		virtual void SetLineWidth(float width) = 0;

		virtual float GetTime() = 0;

		static API GetAPI() { return _sAPI; }

	private:
		static API _sAPI;
	};
}
