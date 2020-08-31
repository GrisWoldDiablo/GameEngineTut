#include "hzpch.h"
#include "OpenGLBuffer.h"

#include <glad/glad.h>

namespace Hazel
{
	// -- VertexBuffer --------------------------
	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
	{
		HZ_PROFILE_FUNCTION();

		glGenBuffers(1, &_rendererID);
		glBindBuffer(GL_ARRAY_BUFFER, _rendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		HZ_PROFILE_FUNCTION();

		glDeleteBuffers(1, &_rendererID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		HZ_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, _rendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		HZ_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	// -----------------------------------------

	// -- IndexBuffer --------------------------
	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
		:_count(count)
	{
		HZ_PROFILE_FUNCTION();

		glGenBuffers(1, &_rendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		HZ_PROFILE_FUNCTION();

		glDeleteBuffers(1, &_rendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		HZ_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		HZ_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	// -----------------------------------------
}
