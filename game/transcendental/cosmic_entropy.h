#pragma once
#include "augs/misc/bit_stream.h"
#include "augs/window_framework/event.h"
#include <map>
#include "game/transcendental/entity_id.h"

namespace augs {
	struct machine_entropy;
}

class cosmos;

struct cosmic_entropy {
	std::map<entity_id, std::vector<augs::window::event::state>> entropy_per_entity;
	augs::bit_stream delta_to_apply;

	// here will be remote entropy as well
	size_t length() const;

	void from_input_receivers_distribution(const augs::machine_entropy&, cosmos&);

	cosmic_entropy& operator+=(const cosmic_entropy&);
};
