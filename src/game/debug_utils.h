#pragma once
#include "augs/log.h"
#include "augs/string/typesafe_sprintf.h"

template <class E>
void warning_other(const E handle, const std::string& content) {
	const auto subject_info = 
		handle.get_name() 
		+ typesafe_sprintf("'s (g: %x)", handle.get_guid().value)
	;

	LOG("WARNING! %x %x.", subject_info, content); 
}

template <class E>
void warning_unset_field(E&& handle, std::string field_name) {
	warning_other(
		std::forward<E>(handle),
	   	typesafe_sprintf("%x was not set", field_name)
	); 
}
