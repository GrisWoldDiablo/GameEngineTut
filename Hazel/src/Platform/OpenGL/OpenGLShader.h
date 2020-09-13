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

		void SetInt(std::string name, int value) override;
		void SetIntArray(const std::string name, int* values, uint32_t count) override;

		void SetFloat(std::string name, float value) override;
		void SetFloat2(std::string name, const glm::vec2& value) override;
		void SetFloat3(std::string name, const glm::vec3& value) override;
		void SetFloat4(std::string name, const glm::vec4& value) override;

		void SetMat4(std::string name, const glm::mat4& value) override;

		void UploadUniformInt(std::string name, int value);
		void UploadUniformIntArray(std::string name, int* values, uint32_t count);

		void UploadUniformFloat(std::string name, float value);
		void UploadUniformFloat2(std::string name, const glm::vec2& value);
		void UploadUniformFloat3(std::string name, const glm::vec3& value);
		void UploadUniformFloat4(std::string name, const glm::vec4& value);

		void UploadUniformMat3(std::string name, const glm::mat3& matrix);
		void UploadUniformMat4(std::string name, const glm::mat4& matrix);

	private:
		std::string ReadFile(const std::string& filePath);
		std::unordered_map<GLenum,std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);

	private:
		uint32_t _rendererID;
		std::string _name;
	};
}
