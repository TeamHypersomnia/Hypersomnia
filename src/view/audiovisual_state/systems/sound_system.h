#pragma once
#include <unordered_map>

#include "augs/misc/delta.h"

#include "augs/audio/sound_source.h"
#include "augs/math/camera_cone.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"

#include "game/components/sound_existence_component.h"

#include "view/viewables/all_viewables_declarations.h"

class interpolation_system;
class cosmos;

namespace augs {
	struct audio_volume_settings;
}

class sound_system {
public:
	struct cache {
		components::sound_existence recorded_component;
		bool constructed = false;
		augs::sound_source source;
	};

	std::unordered_map<entity_id, cache> per_entity_cache;

	cache& get_cache(const const_entity_handle);
	const cache& get_cache(const const_entity_handle) const;

	std::vector<augs::sound_source> fading_sources;

	void reserve_caches_for_entities(const std::size_t) const {}

	void play_nearby_sound_existences(
		const augs::audio_volume_settings&,
		const loaded_sounds&,
		const camera_cone,
		const entity_id listening_character,
		const cosmos&, 
		const interpolation_system& sys,
		const augs::delta dt
	);

	void erase_caches_for_dead_entities(const cosmos&);
};