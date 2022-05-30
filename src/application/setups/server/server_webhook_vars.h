#pragma once
#include "augs/misc/constant_size_string.h"
#include "augs/misc/constant_size_vector.h"

struct server_webhook_vars {
	// GEN INTROSPECTOR struct server_webhook_vars
	augs::constant_size_vector<augs::constant_size_string<400>, 5> duel_of_honor_pics;
	// END GEN INTROSPECTOR

	bool operator==(const server_webhook_vars&) const = default;
};
