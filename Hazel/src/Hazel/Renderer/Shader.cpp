#include "hzpch.h"
#include "Shader.h"

#include <glad/glad.h>

#include "glm/gtc/type_ptr.hpp"

namespace Hazel
{
	Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		// This was taken from OpenGL manual.
		// Shader Compilation
		// https://www.khronos.org/opengl/wiki/Shader_Compilation#Example

		// Create an empty vertex shader handle
		auto vertexShader = glCreateShader(GL_VERTEX_SHADER);

		// Send the vertex shader source code to GL
		// Note that std::string's .c_str is NULL character terminated.
		const auto* source = vertexSrc.c_str();
		glShaderSource(vertexShader, 1, &source, nullptr);

		// Compile the vertex shader
		glCompileShader(vertexShader);

		auto isCompiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			auto maxLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
			glDeleteShader(vertexShader);

			// Use the infoLog as you see fit.
			HZ_CORE_LERROR("{0}", infoLog.data());
			HZ_CORE_ASSERT(false, "Vertex shader compilation failure!");
			// In this simple program, we'll just leave
			return;
		}

		// Create an empty fragment shader handle
		auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		// Send the fragment shader source code to GL
		// Note that std::string's .c_str is NULL character terminated.
		source = fragmentSrc.c_str();
		glShaderSource(fragmentShader, 1, &source, nullptr);

		// Compile the fragment shader
		glCompileShader(fragmentShader);

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			auto maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
			glDeleteShader(fragmentShader);
			// Either of them. Don't leak shaders.
			glDeleteShader(vertexShader);

			// Use the infoLog as you see fit.
			HZ_CORE_LERROR("{0}", infoLog.data());
			HZ_CORE_ASSERT(false, "Fragment shader compilation failure!");
			// In this simple program, we'll just leave
			return;
		}

		// Vertex and fragment shaders are successfully compiled.
		// Now time to link them together into a program.
		// Get a program object.
		_rendererID = glCreateProgram();

		// Attach our shaders to our program
		glAttachShader(_rendererID, vertexShader);
		glAttachShader(_rendererID, fragmentShader);

		// Link our program
		glLinkProgram(_rendererID);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		auto isLinked = 0;
		glGetProgramiv(_rendererID, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			auto maxLength = 0;
			glGetProgramiv(_rendererID, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(_rendererID, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(_rendererID);
			// Don't leak shaders either.
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			// Use the infoLog as you see fit.
			HZ_CORE_LERROR("{0}", infoLog.data());
			HZ_CORE_ASSERT(false, "Shader link failure!");
			// In this simple program, we'll just leave
			return;
		}

		// Always detach shaders after a successful link.
		glDetachShader(_rendererID, vertexShader);
		glDetachShader(_rendererID, fragmentShader);
	}

	Shader::~Shader()
	{
		glDeleteProgram(_rendererID);
	}

	void Shader::Bind() const
	{
		glUseProgram(_rendererID);
	}

	void Shader::Unbind() const
	{
		glUseProgram(0);
	}

	void Shader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
}
