#pragma once

extern "C" // Forward declare of class from C
{
	typedef struct _MonoClassField MonoClassField;
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

		Vector2, Vector3, Vector4,
		Color,
		Entity
	};

	struct ScriptField
	{
		ScriptFieldType Type;
		std::string Name;

		MonoClassField* MonoClassField;
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
			static_assert(sizeof(T) <= 8, "Type too large!");

			return *reinterpret_cast<const T*>(_dataBuffer);
		}

		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 8, "Type too large!");

			memcpy(_dataBuffer, &value, sizeof(T));
		}

	private:
		uint8_t _dataBuffer[8];

		friend class ScriptEngine;
	};

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;
}
