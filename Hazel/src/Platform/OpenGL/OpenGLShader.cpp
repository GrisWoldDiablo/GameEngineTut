#include "hzpch.h"
#include "OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>

#include "glm/gtc/type_ptr.hpp"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "Hazel/Core/Timer.h"

namespace Hazel
{
	namespace Utils
	{
		static GLenum ShaderTypeFromString(const std::string& type)
		{
			HZ_PROFILE_FUNCTION();

			if (type == "vertex")
			{
				return GL_VERTEX_SHADER;
			}
			if (type == "fragment" || type == "pixel")
			{
				return GL_FRAGMENT_SHADER;
			}

			HZ_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER: return shaderc_glsl_vertex_shader;
			case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
			}

			HZ_CORE_ASSERT(false, "Unknown shader type!");
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER: return "GL_VERTEX_SHADER";
			case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			}

			HZ_CORE_ASSERT(false, "Unknown shader type!");
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			// TODO make sure the assets directory is valid
			return "assets/cache/shader/opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
			{
				std::filesystem::create_directories(cacheDirectory);
			}
		}

		static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER: return ".cached_opengl.vert";
			case GL_FRAGMENT_SHADER: return ".cached_opengl.frag";
			}

			HZ_CORE_ASSERT(false, "Unknown stage type!");
			return nullptr;
		}

		static const char* GLShaderStageCachedVulkaFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER: return ".cached_vulkan.vert";
			case GL_FRAGMENT_SHADER: return ".cached_vulkan.frag";
			}

			HZ_CORE_ASSERT(false, "Unknown stage type!");
			return nullptr;
		}
	}

	OpenGLShader::OpenGLShader(const std::string& filePath, bool shouldRecompile)
		:_filePath(filePath)
	{
		HZ_PROFILE_FUNCTION();

		_shouldRecompile = shouldRecompile;

		Utils::CreateCacheDirectoryIfNeeded();
		auto source = ReadFile(filePath);
		auto shaderSources = PreProcess(source);

		HZ_CORE_LINFO("--- Preparing Shaders ---");
		{
			Timer timer;
			_isLoadingCompleted = false;
			_isProgramCreated = false;
			CompileOrGetVulkanBinaries(shaderSources);
			CompileOrGetOpenGLBinaries();
			_isLoadingCompleted = true;
			if (timer.ElapsedMillis() < 1000.0f)
			{
				HZ_CORE_LWARN("Shader creation took {0} milliseconds.", timer.ElapsedMillis());
			}
			else
			{
				HZ_CORE_LWARN("Shader creation took {0} seconds.", timer.Elapsed());
			}
		}

		auto lastSlash = filePath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filePath.rfind('.');
		auto count = lastDot == std::string::npos ? filePath.size() - lastSlash : lastDot - lastSlash;
		_name = filePath.substr(lastSlash, count);
	}

	OpenGLShader::OpenGLShader(std::string name, const std::string& vertexSrc, const std::string& fragmentSrc)
		:_name(std::move(name))
	{
		HZ_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;

		CompileOrGetVulkanBinaries(sources);
		CompileOrGetOpenGLBinaries();
		CreateProgram();
	}

	OpenGLShader::~OpenGLShader()
	{
		HZ_PROFILE_FUNCTION();

		glDeleteProgram(_rendererID);
	}

	std::string OpenGLShader::ReadFile(const std::string& filePath)
	{
		HZ_PROFILE_FUNCTION();

		std::string result;
		std::ifstream inputFileStream(filePath, std::ios::in | std::ios::binary);
		if (!inputFileStream.bad())
		{
			inputFileStream.seekg(0, std::ios::end);
			result.resize(inputFileStream.tellg());
			inputFileStream.seekg(0, std::ios::beg);
			inputFileStream.read(&result[0], result.size());
		}
		else
		{
			HZ_CORE_LERROR("Could not open file {0}", filePath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		HZ_PROFILE_FUNCTION();

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
			HZ_CORE_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specified!");

			size_t nextLinePosition = source.find_first_not_of("\r\n", endOfLine);
			HZ_CORE_ASSERT(nextLinePosition != std::string::npos, "Syntax error!");

			position = source.find(typeToken, nextLinePosition);
			shaderSources[Utils::ShaderTypeFromString(type)] = (position == std::string::npos) ?
				source.substr(nextLinePosition) : source.substr(nextLinePosition, position - nextLinePosition);
		}

		return shaderSources;
	}

	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		HZ_PROFILE_FUNCTION();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

		const bool optimize = true;
		if (optimize)
		{
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		auto& shaderData = _vulkanSPIRV;
		shaderData.clear();
		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = _filePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkaFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (!_shouldRecompile && in.is_open())
			{
				HZ_CORE_LTRACE("Loading Vulkan binary for {0}", Utils::GLShaderStageToString(stage));
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				HZ_CORE_LTRACE("Compiling Vulkan binary for {0}", Utils::GLShaderStageToString(stage));
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), _filePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					HZ_CORE_LERROR(module.GetErrorMessage());
					HZ_CORE_ASSERT(false, "Compilation failed.");
					return;
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : shaderData)
		{
			Reflect(stage, data);
		}
	}

	void OpenGLShader::CompileOrGetOpenGLBinaries()
	{
		HZ_PROFILE_FUNCTION();

		auto& shaderData = _openGLSPIRV;

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);

		const bool optimize = true;
		if (optimize)
		{
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		shaderData.clear();
		_openGLSourceCode.clear();
		for (auto&& [stage, spirv] : _vulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = _filePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (!_shouldRecompile && in.is_open())
			{
				HZ_CORE_LTRACE("Loading OpenGL binary for {0}", Utils::GLShaderStageToString(stage));
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				HZ_CORE_LTRACE("Compiling OpenGL binary for {0}", Utils::GLShaderStageToString(stage));
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				_openGLSourceCode[stage] = glslCompiler.compile();
				auto& source = _openGLSourceCode[stage];

				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), _filePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					HZ_CORE_LERROR(module.GetErrorMessage());
					HZ_CORE_ASSERT(false, "Compilation failed.");
					return;
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	void OpenGLShader::CreateProgram()
	{
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : _openGLSPIRV)
		{
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary((GLsizei)1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), (GLsizei)(spirv.size() * sizeof(uint32_t)));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			HZ_CORE_LERROR("Shader linking failed ({0}):\n{1}", _filePath, infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIDs)
			{
				glDeleteShader(id);
			}
		}
		else
		{
			_isProgramCreated = true;
		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		_rendererID = program;
	}

	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		HZ_CORE_LTRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), _filePath);
		HZ_CORE_LTRACE("    {0} Uniform buffer(s)", resources.uniform_buffers.size());
		HZ_CORE_LTRACE("    {0} Resource(s)", resources.sampled_images.size());

		if (!resources.uniform_buffers.empty())
		{
			HZ_CORE_LTRACE("Uniform buffer(s):");
		}

		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			auto bufferSize = compiler.get_declared_struct_size(bufferType);
			auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto memberCount = bufferType.member_types.size();

			HZ_CORE_LTRACE("  {0}", resource.name);
			HZ_CORE_LTRACE("    Size = {0}", bufferSize);
			HZ_CORE_LTRACE("    Binding = {0}", binding);
			HZ_CORE_LTRACE("    Members = {0}", memberCount);
		}
	}

	void OpenGLShader::Bind() const
	{
		HZ_PROFILE_FUNCTION();

		glUseProgram(_rendererID);
	}

	void OpenGLShader::Unbind() const
	{
		HZ_PROFILE_FUNCTION();

		glUseProgram(0);
	}

	void OpenGLShader::SetInt(std::string name, int value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformInt(std::move(name), value);
	}

	void OpenGLShader::SetIntArray(std::string name, int* values, uint32_t count)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformIntArray(std::move(name), values, count);
	}

	void OpenGLShader::SetBool(std::string name, bool value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformInt(std::move(name), value);
	}

	void OpenGLShader::SetFloat(std::string name, float value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformFloat(std::move(name), value);
	}

	void OpenGLShader::SetFloat2(std::string name, const glm::vec2& value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformFloat2(std::move(name), value);
	}

	void OpenGLShader::SetFloat3(std::string name, const glm::vec3& value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformFloat3(std::move(name), value);
	}

	void OpenGLShader::SetFloat4(std::string name, const glm::vec4& value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformFloat4(std::move(name), value);
	}

	void OpenGLShader::SetMat4(std::string name, const glm::mat4& value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformMat4(std::move(name), value);
	}

	void OpenGLShader::UploadUniformInt(std::string name, int value)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(std::string name, int* values, uint32_t count)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(std::string name, float value)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(std::string name, const glm::vec2& value)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(std::string name, const glm::vec3& value)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(std::string name, const glm::vec4& value)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniformMat3(std::string name, const glm::mat3& matrix)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(std::string name, const glm::mat4& matrix)
	{
		auto location = glGetUniformLocation(_rendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::CompleteInitialization()
	{
		if (!_isProgramCreated && _isLoadingCompleted)
		{
			CreateProgram();
		}
	}
}
