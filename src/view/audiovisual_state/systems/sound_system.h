#pragma once
#include <unordered_map>

#include "augs/misc/timing/delta.h"
#include "augs/templates/hash_templates.h"

#include "augs/math/camera_cone.h"
#include "augs/audio/sound_source_proxy.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/step_declaration.h"

#include "game/detail/view_input/sound_effect_input.h"

#include "view/audiovisual_state/systems/audiovisual_cache_common.h"
#include "view/viewables/all_viewables_declaration.h"
#include "augs/misc/timing/stepped_timing.h"

#include "view/audiovisual_state/systems/sound_system_settings.h"
#include "view/audiovisual_state/systems/sound_system_types.h"
#include "augs/audio/sound_sizes.h"

#include "augs/misc/simple_id_pool.h"

struct character_camera;
class interpolation_system;
class cosmos;

namespace augs {
	struct audio_volume_settings;
	class audio_renderer;
}

struct collision_cooldown_key {
	collision_sound_source participants;
	assets::sound_id id;

	bool operator==(const collision_cooldown_key& b) const {
		return participants == b.participants && id == b.id;
	}
};

namespace std {
	template <>
	struct hash<collision_cooldown_key> {
		std::size_t operator()(const collision_cooldown_key& v) const {
			return hash_multiple(v.participants.subject, v.participants.collider, v.id);
		}
	};
}

class sound_system {
	struct update_properties_input {
		const augs::audio_renderer& renderer;
		sound_system& owner;
		const augs::audio_volume_settings& volume;
		const sound_system_settings& settings;
		const loaded_sounds_map& manager;
		const interpolation_system& interp;
		const character_camera& ear;
		const camera_cone camera;
		const augs::delta dt;
		const double speed_multiplier;
		const double inv_tickrate;
		const double interpolation_ratio;

		const cosmos& get_cosmos() const;
		const_entity_handle get_listener() const;
		std::optional<transformr> find_transform(const absolute_or_local&) const;

		bool under_short_sound_limit() const;
	};

	struct effect_not_found {};

	struct generic_sound_cache {
		float elapsed_secs = 0.f;

		augs::sound_source_proxy_data source;
		packaged_sound_effect original;
		absolute_or_local positioning;

		/* For calculating sound's velocity */
		std::optional<transformr> previous_transform;
		augs::stepped_timestamp when_set_velocity;

		sound_effect_input_vector followup_inputs;

		generic_sound_cache() = default;

		generic_sound_cache(
			augs::sound_source_proxy_id,
			const packaged_sound_effect& original,
			update_properties_input
		); 

		generic_sound_cache(
			augs::sound_source_proxy_id,
			const packaged_multi_sound_effect& original,
			update_properties_input
		); 

		bool should_play(update_properties_input in) const;
		bool rebind_buffer(update_properties_input in);
		void update_properties(update_properties_input in);

		augs::sound_source_proxy get_proxy(const update_properties_input& in);
		void stop_and_free(const update_properties_input& in);
		void bind(const update_properties_input&, const augs::sound_buffer&);

		bool probably_still_playing() const;
		void maybe_play_next(update_properties_input in);

	private:
		void eat_followup();
		void init(update_properties_input);
	};

	struct fading_source {
		assets::sound_id id;
		augs::sound_source_proxy_data source;
		float fade_per_sec = 3.f;
	};

	struct collision_sound_cooldown {
		float remaining_ms = 0.f;
		int consecutive_occurences = 0;
	};

	struct damage_sound_cooldown {
		float current_ms = 0.f;
		float max_ms = 0.f;

		int consecutive_occurences = 0;
	};

	augs::constant_size_vector<generic_sound_cache, SOUNDS_SOURCES_IN_POOL> short_sounds;

	struct recorded_meta {
		std::string name;
	};

	struct continuous_sound_cache {
		generic_sound_cache cache;
		recorded_meta recorded;
	};

	augs::simple_id_pool<augs::constant_size_vector<augs::sound_source_proxy_id, SOUNDS_SOURCES_IN_POOL>> id_pool = SOUNDS_SOURCES_IN_POOL;

	audiovisual_cache_map<continuous_sound_cache> firearm_engine_caches;
	audiovisual_cache_map<continuous_sound_cache> continuous_sound_caches;

	augs::constant_size_vector<fading_source, MAX_FADING_SOURCES> fading_sources;
	std::unordered_map<collision_cooldown_key, collision_sound_cooldown> collision_sound_cooldowns;
	std::unordered_map<collision_cooldown_key, damage_sound_cooldown> damage_sound_cooldowns;

	template <class T>
	void fade_and_erase(const update_properties_input& in, T& caches, const unversioned_entity_id id, const float fade_per_sec = 3.f) {
		if (auto* const cache = mapped_or_nullptr(caches, id)) {
			if (!start_fading(cache->cache, fade_per_sec)) {
				cache->cache.stop_and_free(in);
			}

			caches.erase(id);
		}
	}

	void update_listener(
		const augs::audio_renderer& renderer,
		const character_camera& listener,
		const interpolation_system& sys,
		const sound_system_settings& settings,
		const vec2 world_screen_center
	);

	bool start_fading(generic_sound_cache&, float fade_per_sec = 3.f);

	float after_flash_passed_ms = 0.f;
	float last_registered_flash_mult = 0.f;

	float silent_trace_cooldown = 0.f;
	int current_num_silent_traces = 0;

public:
	void reserve_caches_for_entities(const std::size_t) const {}

	void update_effects_from_messages(const_logic_step, update_properties_input);
	void update_sound_properties(update_properties_input);

	void fade_sources(
		const augs::audio_renderer& renderer,
		const augs::delta dt
	);

	void clear();
	void clear_sources_playing(const assets::sound_id);

	void advance_flash(const_entity_handle listener, augs::delta dt);

	auto get_effective_flash_mult() const {
		return last_registered_flash_mult;
	}
};