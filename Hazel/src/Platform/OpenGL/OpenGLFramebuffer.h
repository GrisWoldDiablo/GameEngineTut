#pragma once
#include "Hazel/Renderer/Framebuffer.h"

namespace Hazel
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer() override;

		void Invalidate();

		void Bind() override;
		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;
		int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		void ClearAttachment(uint32_t attachmentIndex, int value) override;

		uint32_t GetColorAttachmentRenderID(uint32_t index = 0) const override { HZ_CORE_ASSERT(index < _colorAttachments.size(), "Index out of range"); return _colorAttachments[index]; }
		const FramebufferSpecification& GetSpecification() const override { return _specification; }

	private:
		void Clean();

	private:
		uint32_t _rendererID = 0;
		FramebufferSpecification _specification;

		std::vector<FramebufferTextureSpecification> _colorAttachmentSpecifications;
		std::vector<uint32_t> _colorAttachments;

		FramebufferTextureSpecification _depthAttachmentSpecification = FramebufferTextureFormat::None;
		uint32_t _depthAttachment = 0;
	};
}
