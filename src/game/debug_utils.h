#pragma once
#include "augs/log.h"
#include "augs/templates/string_templates.h"
#include "augs/misc/typesafe_sprintf.h"

template <class E>
void warning_unset_field(const E handle, std::string field_name) {
	const auto subject_info = 
		to_string(handle.get_name()) 
		+ typesafe_sprintf("'s (g: %x)", handle.get_guid().value)
	;

	LOG("WARNING! %x %x was not set.", subject_info, field_name); 
}
