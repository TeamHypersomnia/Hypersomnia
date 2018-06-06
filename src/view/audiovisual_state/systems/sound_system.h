#pragma once
#include <unordered_map>

#include "augs/misc/timing/delta.h"

#include "augs/audio/sound_source.h"
#include "augs/math/camera_cone.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"

#include "game/detail/view_input/sound_effect_input.h"

#include "view/audiovisual_state/systems/audiovisual_cache_common.h"
#include "view/viewer_eye.h"
#include "view/viewables/all_viewables_declarations.h"

class interpolation_system;
class cosmos;

namespace augs {
	struct audio_volume_settings;
}

class sound_system {
	struct generic_sound_cache {
		augs::sound_source source;
		absolute_or_local positioning;

		sound_effect_input original_effect;
		sound_effect_start_input original_start;

		/* For calculating sound's velocity */
		std::optional<components::transform> previous_transform;

		generic_sound_cache() = default;

		generic_sound_cache(
			const_entity_handle listening_character,
			const augs::sound_buffer& source_effect,
			const sound_effect_input&,
			const sound_effect_start_input&,
			const interpolation_system&
		);

		bool update_properties(
			const augs::audio_volume_settings&,
			const cosmos&,
			const interpolation_system&,
			const vec2 listener_pos,
			const augs::delta dt
		);
	};

	struct fading_source {
		assets::sound_id id;
		augs::sound_source source;
	};

	std::vector<generic_sound_cache> short_sounds;
	std::vector<fading_source> fading_sources;

	audiovisual_cache_map<generic_sound_cache> firearm_engine_caches;

	void update_listener(
		const const_entity_handle subject,
		const interpolation_system& sys
	);

	void start_fading(generic_sound_cache&);

public:
	void reserve_caches_for_entities(const std::size_t) const {}

	void update_effects_from_messages(
		const_logic_step step,
		const loaded_sounds_map& manager,
		const interpolation_system& interp,
		viewer_eye ear
	);

	void update_sound_properties(
		const augs::audio_volume_settings&,
		const loaded_sounds_map&,
		const interpolation_system& sys,
		viewer_eye ear,
		augs::delta dt
	);

	void fade_sources(const augs::delta dt);

	void clear();
	void clear_sources_playing(const assets::sound_id);
	void clear_dead_entities(const cosmos&);

	//	void set_listening_character(entity_id);
};