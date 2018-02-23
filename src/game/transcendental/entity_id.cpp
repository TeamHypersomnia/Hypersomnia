#include "game/transcendental/entity_id.h"
#include "augs/templates/string_templates.h"

std::ostream& operator<<(std::ostream& out, const entity_id x) {
	return out << get_type_name(x.type_id) << "-" << x.basic();
}

