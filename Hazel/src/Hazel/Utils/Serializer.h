#pragma once

#include "Hazel/Scripting/ScriptField.h"

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<std::filesystem::path>
	{
		static Node encode(const std::filesystem::path& rhs)
		{
			return Node(rhs.string());
		}

		static bool decode(const Node& node, std::filesystem::path& rhs)
		{
			if (!node.IsScalar())
			{
				return false;
			}

			rhs = node.Scalar();
			return true;
		}
	};

	template<>
	struct convert<Hazel::ScriptFieldType>
	{
		static Node encode(const Hazel::ScriptFieldType& rhs)
		{
			return Node(Hazel::Utils::ScriptFieldTypeToString(rhs));
		}

		static bool decode(const Node& node, Hazel::ScriptFieldType& rhs)
		{
			if (!node.IsScalar())
			{
				return false;
			}

			rhs = Hazel::Utils::ScriptFieldTypeFromString(node.Scalar());
			return true;
		}
	};
}

namespace Hazel
{
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const std::filesystem::path& path)
	{
		return out.Write(path.string());
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const ScriptFieldType& type)
	{
		return out.Write(Utils::ScriptFieldTypeToString(type));
	}

	class Serializer
	{
	public:
		virtual ~Serializer() = default;

		virtual void Serialize(const std::filesystem::path& filepath) = 0;
		virtual void SerializeRuntime() = 0;

		virtual bool Deserialize(const std::filesystem::path& filepath, bool isWithLog = true) = 0;
		virtual bool DeserializeRuntime() = 0;
	};
}
