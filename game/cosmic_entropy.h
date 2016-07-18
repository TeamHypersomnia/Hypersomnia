#pragma once
#include "augs/window_framework/event.h"
#include <map>
#include "game/entity_id.h"

struct cosmic_entropy {
	std::map<entity_id, std::vector<augs::window::event::state>> entropy_per_entity;
	// here will be remote entropy as well
	size_t length() const;

	cosmic_entropy& operator+=(const cosmic_entropy&);
};
