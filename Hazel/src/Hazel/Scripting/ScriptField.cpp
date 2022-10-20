#include "hzpch.h"
#include "ScriptField.h"

#include "mono/metadata/object.h"

namespace Hazel
{
	// TODO? I dont like this, maybe save the value at construction?
	std::string ScriptField::GetFieldTypeName() const
	{
		return mono_type_get_name(mono_field_get_type(MonoClassField));
	}
}
