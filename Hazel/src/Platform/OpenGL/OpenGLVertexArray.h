#pragma once

#include "Hazel/Renderer/VertexArray.h"

namespace Hazel
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray() override;

		void Bind() const override;
		void Unbind() const override;

		void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;
		
		const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return _vertexBuffers; };
		const Ref<IndexBuffer>& GetIndexBuffer() const override { return _indexBuffer; };
		
	private:
		uint32_t _rendererID;
		std::vector<Ref<VertexBuffer>> _vertexBuffers;
		uint32_t _vertexBufferIndex = 0;
		Ref<IndexBuffer> _indexBuffer;
	};
}
