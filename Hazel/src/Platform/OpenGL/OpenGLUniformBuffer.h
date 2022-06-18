#pragma once
#include "Hazel/Renderer/UniformBuffer.h"

namespace Hazel
{
	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~OpenGLUniformBuffer() override;


		void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

	private:
		uint32_t _rendererID = 0;
	};
}
