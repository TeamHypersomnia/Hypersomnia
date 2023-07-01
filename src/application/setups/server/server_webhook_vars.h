#pragma once
#include "augs/network/network_types.h"

struct server_webhook_vars {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct server_webhook_vars
	address_string_type duel_of_honor_pic_link_pattern = "https://hypersomnia.xyz/duels/%x.jpg";
	address_string_type duel_victory_pic_link_pattern = "https://hypersomnia.xyz/duels/victory.jpg";
	address_string_type fled_pic_link = "https://hypersomnia.xyz/duels/shameful.jpg";
	address_string_type reconsidered_pic_link = "https://hypersomnia.xyz/duels/reconsidered.jpg";
	uint32_t num_duel_pics = 6;
	// END GEN INTROSPECTOR

	bool operator==(const server_webhook_vars&) const = default;
};
