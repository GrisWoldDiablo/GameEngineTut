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

		void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;
		
		const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return _vertexBuffers; };
		const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const override { return _indexBuffer; };
		
	private:
		uint32_t _rendererID;
		std::vector<std::shared_ptr<VertexBuffer>> _vertexBuffers;
		std::shared_ptr<IndexBuffer> _indexBuffer;
	};
}
