#pragma once
#include <string>

#include <glm/glm.hpp>

namespace Hazel
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual const std::string& GetName() = 0;

		virtual void SetInt(std::string name, int value) = 0;
		virtual void SetIntArray(std::string name, int* values, uint32_t count) = 0;
		virtual void SetBool(std::string name, bool value) = 0;

		virtual void SetFloat(std::string name, float value) = 0;
		virtual void SetFloat2(std::string name, const glm::vec2& value) = 0;
		virtual void SetFloat3(std::string name, const glm::vec3& value) = 0;
		virtual void SetFloat4(std::string name, const glm::vec4& value) = 0;

		virtual void SetMat4(std::string name, const glm::mat4& value) = 0;

		static Ref<Shader> Create(const std::string& filePath, bool shouldRecompile = false);
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);

		virtual void CompleteInitialization() = 0;
		bool IsLoadingCompleted() const { return _isLoadingCompleted; }

	protected:
		bool _isLoadingCompleted;
		bool _shouldRecompile;
	};

	class ShaderLibrary
	{
	public:
		void Add(const std::string& name, const Ref<Shader>& shader);
		void Add(const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filePath);
		Ref<Shader> Load(const std::string& name, const std::string& filePath);

		Ref<Shader> Get(const std::string& name);

		bool Exist(const std::string& name) const;

	private:
		std::unordered_map<std::string, Ref<Shader>> _shaders;
	};
}
