#include "hzpch.h"
#include "ScriptField.h"

#include "mono/metadata/class.h"

namespace Hazel
{
	// TODO? I don't like this, maybe save the value at construction?
	std::string ScriptField::GetFieldTypeName() const
	{
		return mono_type_get_name(mono_field_get_type(MonoClassField));
	}

	MonoClass* ScriptField::GetFieldTypeClass() const
	{
		return mono_type_get_class(mono_field_get_type(MonoClassField));
	}
}
