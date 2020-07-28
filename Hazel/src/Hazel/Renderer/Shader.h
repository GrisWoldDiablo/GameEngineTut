#pragma once
#include <string>

namespace Hazel
{
	class Shader
	{
	public:
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
		~Shader();

		void Bind() const;
		void Unbind() const;
		uint32_t GetRendererID() { return _rendererID; }
	private:
		uint32_t _rendererID;
	};
}
