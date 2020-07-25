#pragma once

namespace Hazel
{
	enum class RendererAPI
	{
		None = 0,
		OpenGL = 1
	};

	class Renderer
	{
	public:
		inline static RendererAPI GetAPI() { return _sRendererAPI; }

	private:
		static RendererAPI _sRendererAPI;
	};
	
}
