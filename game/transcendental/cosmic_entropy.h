#pragma once
#include "augs/misc/streams.h"
#include "augs/window_framework/event.h"
#include <map>
#include "game/transcendental/entity_id.h"
#include "game/messages/intent_message.h"

namespace augs {
	struct machine_entropy;
}

class cosmos;
struct input_context;

struct cosmic_entropy {
	std::map<entity_id, std::vector<entity_intent>> entropy_per_entity;
	augs::stream delta_to_apply;

	// here will be remote entropy as well
	size_t length() const;

	void from_input_receivers_distribution(const augs::machine_entropy&, const input_context&, cosmos&);
	static bool make_intent(const input_context&, const augs::window::event::state&, entity_intent& mapped_intent);

	cosmic_entropy& operator+=(const cosmic_entropy&);
};
