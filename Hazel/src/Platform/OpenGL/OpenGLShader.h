#pragma once
#include "Hazel/Renderer/Shader.h"
#include <glm/glm.hpp>

// #TODO: Remove!
typedef unsigned int GLenum;

namespace Hazel
{
	class OpenGLShader :public Shader
	{
	public:
		OpenGLShader(const std::string& filePath);
		OpenGLShader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc);
		~OpenGLShader() override;

		void Bind() const override;
		void Unbind() const override;

		const std::string& GetName() override { return _name; };

		void SetInt(const std::string name, int value) override;

		void SetFloat2(const std::string name, const glm::vec2& value) override;
		void SetFloat3(const std::string name, const glm::vec3& value) override;
		void SetFloat4(const std::string name, const glm::vec4& value) override;

		void SetMat4(const std::string name, const glm::mat4& value) override;

		void UploadUniformInt(const std::string& name, int value);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value);

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		std::string ReadFile(const std::string& filePath);
		std::unordered_map<GLenum,std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);

	private:
		uint32_t _rendererID;
		std::string _name;
	};
}
