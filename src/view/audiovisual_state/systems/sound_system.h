#pragma once
#include <unordered_map>

#include "augs/misc/timing/delta.h"

#include "augs/audio/sound_source.h"
#include "augs/math/camera_cone.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/step_declaration.h"

#include "game/detail/view_input/sound_effect_input.h"

#include "view/audiovisual_state/systems/audiovisual_cache_common.h"
#include "view/viewables/all_viewables_declaration.h"

struct character_camera;
class interpolation_system;
class cosmos;

namespace augs {
	struct audio_volume_settings;
}

class sound_system {
	struct update_properties_input {
		const augs::audio_volume_settings& settings;
		const loaded_sounds_map& manager;
		const interpolation_system& interp;
		const character_camera& ear;
		const augs::delta dt;

		const_entity_handle get_listener() const;
		std::optional<transformr> find_transform(const absolute_or_local&) const;
	};

	struct effect_not_found {};

	struct generic_sound_cache {
		augs::sound_source source;
		packaged_sound_effect original;
		absolute_or_local positioning;

		/* For calculating sound's velocity */
		std::optional<transformr> previous_transform;

		generic_sound_cache() = default;
		generic_sound_cache(
			const packaged_sound_effect& original,
			const update_properties_input in
		) : 
			original(original),
			positioning(original.start.positioning)
		{
			if (!rebind_buffer(in)) {
				throw effect_not_found {}; 
			}

			update_properties(in);
			previous_transform = in.find_transform(positioning);
			source.play();
		}

		bool rebind_buffer(update_properties_input in);
		bool update_properties(update_properties_input in);

		void bind(const augs::sound_buffer&);
	};

	struct fading_source {
		assets::sound_id id;
		augs::sound_source source;
		float fade_per_sec = 3.f;
	};

	std::vector<generic_sound_cache> short_sounds;
	audiovisual_cache_map<generic_sound_cache> firearm_engine_caches;
	audiovisual_cache_map<generic_sound_cache> continuous_sound_caches;

	std::vector<fading_source> fading_sources;

	template <class T>
	void fade_and_erase(T& caches, const unversioned_entity_id id, const float fade_per_sec = 3.f) {
		if (auto* const cache = mapped_or_nullptr(caches, id)) {
			start_fading(*cache, fade_per_sec);
			caches.erase(id);
		}
	}

	void update_listener(
		const const_entity_handle subject,
		const interpolation_system& sys
	);

	void start_fading(generic_sound_cache&, float fade_per_sec = 3.f);

public:
	void reserve_caches_for_entities(const std::size_t) const {}

	void update_effects_from_messages(const_logic_step, update_properties_input);
	void update_sound_properties(update_properties_input);

	void fade_sources(const augs::delta dt);

	void clear();
	void clear_sources_playing(const assets::sound_id);
	void clear_dead_entities(const cosmos&);

	//	void set_listening_character(entity_id);
};