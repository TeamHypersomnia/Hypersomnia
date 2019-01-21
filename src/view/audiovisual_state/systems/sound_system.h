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
#include "augs/misc/timing/stepped_timing.h"

#include "view/audiovisual_state/systems/sound_system_settings.h"

struct character_camera;
class interpolation_system;
class cosmos;

namespace augs {
	struct audio_volume_settings;
}

class sound_system {
	struct update_properties_input {
		const augs::audio_volume_settings& volume;
		const sound_system_settings& settings;
		const loaded_sounds_map& manager;
		const interpolation_system& interp;
		const character_camera& ear;
		const augs::delta dt;
		const double speed_multiplier;
		const double inv_tickrate;

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
		augs::stepped_timestamp when_set_velocity;

		sound_effect_input_vector followup_inputs;

		generic_sound_cache() = default;

		generic_sound_cache(
			const packaged_sound_effect& original,
			update_properties_input
		); 

		generic_sound_cache(
			const packaged_multi_sound_effect& original,
			update_properties_input
		); 

		bool should_play(update_properties_input in) const;
		bool rebind_buffer(update_properties_input in);
		void update_properties(update_properties_input in);

		void bind(const augs::sound_buffer&);

		bool still_playing() const;
		void maybe_play_next(update_properties_input in);

	private:
		void eat_followup();
		void init(update_properties_input);
	};

	struct fading_source {
		assets::sound_id id;
		augs::sound_source source;
		float fade_per_sec = 3.f;
	};

	struct collision_sound_cooldown {
		float remaining_ms = 0.f;
		int consecutive_occurences = 0;
	};

	augs::constant_size_vector<generic_sound_cache, MAX_SHORT_SOUNDS> short_sounds;

	struct recorded_meta {
		std::string name;
	};

	struct continuous_sound_cache {
		generic_sound_cache cache;
		recorded_meta recorded;
	};

	audiovisual_cache_map<continuous_sound_cache> firearm_engine_caches;
	audiovisual_cache_map<continuous_sound_cache> continuous_sound_caches;

	augs::constant_size_vector<fading_source, MAX_FADING_SOURCES> fading_sources;
	std::unordered_map<collision_sound_source, collision_sound_cooldown> collision_sound_cooldowns;

	template <class T>
	void fade_and_erase(T& caches, const unversioned_entity_id id, const float fade_per_sec = 3.f) {
		if (auto* const cache = mapped_or_nullptr(caches, id)) {
			start_fading(cache->cache, fade_per_sec);
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

	//	void set_listening_character(entity_id);
};