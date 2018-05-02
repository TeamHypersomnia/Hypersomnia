#include "game/transcendental/entity_id.h"
#include "augs/string/string_templates.h"
#include "augs/string/get_current_type_name.h"

std::ostream& operator<<(std::ostream& out, const entity_id x) {
	if (!x.is_set()) {
		return out << "(unset)";
	}

	return out << get_current_type_name(x.type_id) << "-" << x.basic();
}

