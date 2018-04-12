#include "game/transcendental/entity_id.h"
#include "augs/templates/string_templates.h"
#include "augs/templates/get_current_type_name.h"

std::ostream& operator<<(std::ostream& out, const entity_id x) {
	return out << get_current_type_name(x.type_id) << "-" << x.basic();
}

