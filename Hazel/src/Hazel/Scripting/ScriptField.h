#pragma once

extern "C" // Forward declare of class from C
{
	typedef struct _MonoClassField MonoClassField;
	typedef struct _MonoString MonoString;
}

namespace Hazel
{
	// https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/language-specification/types#836-integral-types
	enum class ScriptFieldType
	{
		None = 0,
		Float, Double, Char, Bool,
		SByte, Short, Int, Long,
		Byte, UShort, UInt, ULong,

		String,

		Vector2, Vector3, Vector4,
		Color,
		Entity
	};

	struct ScriptField
	{
		ScriptFieldType Type;
		std::string Name;
		MonoClassField* MonoClassField;
		uint8_t DefaultData[16];
		std::string DefaultStringData;

		template<typename T>
		T GetDefaultValue() const
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			return *reinterpret_cast<const T*>(DefaultData);
		}

		std::string GetDefaultStringValue() const
		{
			return DefaultStringData;
		}

		std::string GetFieldTypeName() const;
	};

	// ScriptField + data storage
	struct ScriptFieldInstance
	{
		ScriptField Field;

		ScriptFieldInstance()
		{
			memset(_dataBuffer, 0, sizeof(_dataBuffer));
		}

		template<typename T>
		T GetValue() const
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			return *reinterpret_cast<const T*>(_dataBuffer);
		}

		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			memcpy_s(_dataBuffer, sizeof(_dataBuffer), &value, sizeof(T));
		}

		std::string GetStringValue() const
		{
			return _stringData;
		}

		void SetStringValue(const std::string& value)
		{
			_stringData = value;
		}

	private:
		uint8_t _dataBuffer[16];
		std::string _stringData;

		friend class ScriptEngine;
	};

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

	namespace Utils
	{
		inline const std::string ScriptFieldTypeToString(ScriptFieldType scriptFieldType)
		{
			switch (scriptFieldType)
			{
			case ScriptFieldType::None:   return "None";

			case ScriptFieldType::Float:  return "Float";
			case ScriptFieldType::Double: return "Double";
			case ScriptFieldType::Char:	  return "Char";
			case ScriptFieldType::Bool:	  return "Bool";

			case ScriptFieldType::SByte:  return "SByte";
			case ScriptFieldType::Short:  return "Short";
			case ScriptFieldType::Int:	  return "Int";
			case ScriptFieldType::Long:	  return "Long";

			case ScriptFieldType::Byte:	  return "Byte";
			case ScriptFieldType::UShort: return "UShort";
			case ScriptFieldType::UInt:	  return "UInt";
			case ScriptFieldType::ULong:  return "ULong";

			case ScriptFieldType::String: return "String";

			case ScriptFieldType::Vector2:return "Vector2";
			case ScriptFieldType::Vector3:return "Vector3";
			case ScriptFieldType::Vector4:return "Vector4";

			case ScriptFieldType::Color:  return "Color";
			case ScriptFieldType::Entity: return "Entity";
			}

			HZ_CORE_ASSERT(false, "Unknown ScriptFieldType!");
			return "None";
		}

		inline ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
		{
			if (fieldType == "None")	return ScriptFieldType::None;

			if (fieldType == "Float")	return ScriptFieldType::Float;
			if (fieldType == "Double")	return ScriptFieldType::Double;
			if (fieldType == "Char")	return ScriptFieldType::Char;
			if (fieldType == "Bool")	return ScriptFieldType::Bool;

			if (fieldType == "SByte")	return ScriptFieldType::SByte;
			if (fieldType == "Short")	return ScriptFieldType::Short;
			if (fieldType == "Int")		return ScriptFieldType::Int;
			if (fieldType == "Long")	return ScriptFieldType::Long;

			if (fieldType == "Byte")	return ScriptFieldType::Byte;
			if (fieldType == "UShort")	return ScriptFieldType::UShort;
			if (fieldType == "UInt")	return ScriptFieldType::UInt;
			if (fieldType == "ULong")	return ScriptFieldType::ULong;

			if (fieldType == "String")	return ScriptFieldType::String;

			if (fieldType == "Vector2") return ScriptFieldType::Vector2;
			if (fieldType == "Vector3") return ScriptFieldType::Vector3;
			if (fieldType == "Vector4") return ScriptFieldType::Vector4;

			if (fieldType == "Color")	return ScriptFieldType::Color;
			if (fieldType == "Entity")	return ScriptFieldType::Entity;

			HZ_CORE_ASSERT(false, "Unknown ScriptFieldType!");
			return ScriptFieldType::None;
		}
	}
}
