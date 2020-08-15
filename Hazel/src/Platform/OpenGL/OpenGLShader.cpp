#include "hzpch.h"
#include "OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>

#include "glm/gtc/type_ptr.hpp"

namespace Hazel
{
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
		{
			return GL_VERTEX_SHADER;
		}
		if (type == "fragment" || type == "pixel")
		{
			return GL_FRAGMENT_SHADER;
		}

		HZ_CORE_ASSERT(false, "Unknow shader type!");
		return 0;
	}

	OpenGLShader::OpenGLShader(const std::string& glslFilePath)
	{
		auto source = ReadFile(glslFilePath);
		auto shaderSources = PreProcess(source);
		Compile(shaderSources);
		
		auto lastSlash = glslFilePath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = glslFilePath.rfind('.');
		auto count = lastDot == std::string::npos ? glslFilePath.size() - lastSlash : lastDot - lastSlash;
		_name = glslFilePath.substr(lastSlash, count);
	}

	OpenGLShader::OpenGLShader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc)
		:_name(std::move(name))
	{
		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;
		Compile(sources);
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(_rendererID);
	}

	std::string OpenGLShader::ReadFile(const std::string& filePath)
	{
		std::string result;
		std::ifstream inputFileStream(filePath, std::ios::in | std::ios::binary);
		if (!inputFileStream.bad())
		{
			inputFileStream.seekg(0, std::ios::end);
			result.resize(inputFileStream.tellg());
			inputFileStream.seekg(0, std::ios::beg);
			inputFileStream.read(&result[0], result.size());
			inputFileStream.close();
		}
		else
		{
			HZ_CORE_LERROR("Could not open file {0}", filePath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		auto typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t position = source.find(typeToken, 0);
		while (position != std::string::npos)
		{
			size_t endOfLine = source.find_first_of("\r\n", position);
			HZ_CORE_ASSERT(endOfLine != std::string::npos, "Syntax error!");

			size_t begin = position + typeTokenLength + 1;
			auto type = source.substr(begin, endOfLine - begin);
			HZ_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified!");

			size_t nextLinePosition = source.find_first_not_of("\r\n", endOfLine);
			HZ_CORE_ASSERT(nextLinePosition != std::string::npos, "Syntax error!");

			position = source.find(typeToken, nextLinePosition);
			shaderSources[ShaderTypeFromString(type)] = (position == std::string::npos) ? 
				source.substr(nextLinePosition) : source.substr(nextLinePosition,position - nextLinePosition);
		}

		return shaderSources;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();

		HZ_CORE_ASSERT(shaderSources.size() <= 2, "We only support 2 shaders for now");
		std::array<GLenum, 2> glShaderIDs;
		int glShaderIDindex = 0;

		for (auto&& [key, value] : shaderSources)
		{
			GLenum type = key;
			const std::string& source = value;

			auto shader = glCreateShader(type);

			const auto* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, nullptr);

			glCompileShader(shader);

			auto isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				auto maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(shader);

				HZ_CORE_LERROR("{0}", infoLog.data());
				HZ_CORE_ASSERT(false, "Shader compilation failure!");
				break;
			}

			glAttachShader(program, shader);
			glShaderIDs[glShaderIDindex++] = shader;
		}


		glLinkProgram(program);

		auto isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			auto maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(program);

			for (auto id : glShaderIDs)
			{
				glDeleteShader(id);
			}

			HZ_CORE_LERROR("{0}", infoLog.data());
			HZ_CORE_ASSERT(false, "Shader link failure!");
			return;
		}

		// Always detach and delete shaders after a successful link.
		for (auto id : glShaderIDs)
		{
			glDetachShader(_rendererID, id);
			glDeleteShader(id);
		}

		_rendererID = program;
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(_rendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
}
