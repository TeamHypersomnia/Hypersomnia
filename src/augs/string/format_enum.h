#pragma once
#include "augs/templates/enum_introspect.h"
#include "augs/string/string_templates_declaration.h"

template <class Enum>
auto format_enum(const Enum e) {
	return format_field_name(to_lowercase(augs::enum_to_string(e)));
}