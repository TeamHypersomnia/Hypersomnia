#pragma once
#include "augs/misc/constant_size_string.h"
#include "augs/misc/constant_size_vector.h"

struct server_webhook_vars {
	// GEN INTROSPECTOR struct server_webhook_vars
	augs::constant_size_string<400> duel_of_honor_pic_link_pattern = "https://hypersomnia.xyz/duels/%x.jpg";
	uint32_t num_duel_pics = 6;
	// END GEN INTROSPECTOR

	bool operator==(const server_webhook_vars&) const = default;
};
