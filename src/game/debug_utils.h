#pragma once
#include "game/common_state/entity_name_str.h"

void log_warn_other(const entity_name_str&, entity_id_base, const char* c1, const char* c2 = "");

template <class E>
void warning_other(const E& handle, const char* content) {
	const auto id = handle.get_id().raw;
	log_warn_other(handle.get_name(), id, content);
}

template <class E>
void warning_unset_field(E&& handle, const char* field_name) {
	const auto id = handle.get_id().raw;
	log_warn_other(handle.get_name(), id, field_name, " was not set");
}
