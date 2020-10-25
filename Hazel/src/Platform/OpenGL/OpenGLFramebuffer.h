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
		
		uint32_t GetColorAttachmentRenderID() const override { return _colorAttachment; }
		const FramebufferSpecification& GetSpecification() const override { return _specification; }

	private :
		uint32_t _rendererID;
		uint32_t _colorAttachment;
		uint32_t _depthAttachment;
		FramebufferSpecification _specification;
	};
}
