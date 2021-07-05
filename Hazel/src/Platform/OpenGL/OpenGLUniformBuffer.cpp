#include "hzpch.h"
#include "OpenGLUniformBuffer.h"

#include <glad/glad.h>

namespace Hazel
{
	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
	{
		glCreateBuffers(1, &_rendererID);
		glNamedBufferData(_rendererID, size, nullptr, GL_DYNAMIC_DRAW); // TODO investigate usage hint
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, _rendererID);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &_rendererID);
	}

	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(_rendererID, offset, size, data);
	}
}
