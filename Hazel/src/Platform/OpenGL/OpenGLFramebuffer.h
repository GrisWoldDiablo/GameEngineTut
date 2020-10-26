#pragma once
#include "Hazel/Renderer/Framebuffer.h"

namespace Hazel
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		void Invalidate();

		void Bind() override;
		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;
		
		uint32_t GetColorAttachmentRenderID() const override { return _colorAttachment; }
		const FramebufferSpecification& GetSpecification() const override { return _specification; }

	private:
		void Clean();

	private:
		uint32_t _rendererID = 0;
		uint32_t _colorAttachment = 0;
		uint32_t _depthAttachment = 0;
		FramebufferSpecification _specification;
	};
}
