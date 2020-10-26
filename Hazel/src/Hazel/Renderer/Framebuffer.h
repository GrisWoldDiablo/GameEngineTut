#pragma once
#include "Hazel/Core/Base.h"

namespace Hazel
{
	struct FramebufferSpecification
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t Sample = 1;

		bool SwapChainTarget = false; // glBindFramebuffer(0);
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetColorAttachmentRenderID() const = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}
