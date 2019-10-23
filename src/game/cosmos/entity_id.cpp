#include "augs/log.h"
#include "game/common_state/entity_name_str.h"
#include "game/cosmos/entity_id.h"
#include "augs/string/string_templates.h"
#include "augs/string/get_current_type_name.h"

void log_warn_other(const entity_name_str& name, const entity_id_base id, const char* c1, const char* c2) {
	const auto subject_info = 
		name
		+ typesafe_sprintf("'s (id: %x)", id)
	;

	LOG("WARNING! %x %x%x.", subject_info, c1, c2); 
}

std::ostream& operator<<(std::ostream& out, const entity_id x) {
	if (!x.is_set()) {
		return out << "(unset)";
	}

	return out << get_current_type_name(x.type_id) << "-" << x.raw;
}

