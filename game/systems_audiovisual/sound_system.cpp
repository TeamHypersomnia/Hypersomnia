#include "sound_system.h"

#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

void sound_system::resample_state_for_audiovisuals(const cosmos& new_cosmos) {
	std::vector<entity_id> to_erase;

	for (const auto it : per_entity_cache) {
		if (new_cosmos[it.first].dead()) {
			to_erase.push_back(it.first);
		}
	}

	for (const auto it : to_erase) {
		per_entity_cache.erase(it);
	}
}

void sound_system::initialize_sound_sources(const size_t num_max_sources) {
	sources.resize(num_max_sources);
}

sound_system::cache& sound_system::get_cache(const const_entity_handle id) {
	return per_entity_cache[id.get_id()];
}

const sound_system::cache& sound_system::get_cache(const const_entity_handle id) const {
	return per_entity_cache.at(id.get_id());
}