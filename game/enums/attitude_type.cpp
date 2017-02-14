#include "attitude_type.h"

bool is_hostile(const attitude_type att) {
	return
		att == attitude_type::WANTS_TO_KILL
		|| att == attitude_type::WANTS_TO_KNOCK_UNCONSCIOUS;
}