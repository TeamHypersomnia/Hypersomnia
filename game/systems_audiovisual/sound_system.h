#pragma once
#include <array>
#include <unordered_map>

#include "game/components/sound_existence_component.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"

#include "augs/misc/delta.h"

class viewing_step;

class sound_system {
public:
	struct cache {
		components::sound_existence recorded_component;
		bool constructed = false;
	};

	std::unordered_map<entity_id, cache> per_entity_cache;

	cache& get_cache(const const_entity_handle);
	const cache& get_cache(const const_entity_handle) const;

	void reserve_caches_for_entities(const size_t) const {}

	void resample_state_for_audiovisuals(const cosmos&);
};