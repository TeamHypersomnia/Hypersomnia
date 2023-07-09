#pragma once
#include "augs/entity_system/storage_for_systems.h"
#include "augs/math/camera_cone.h"

#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_id.h"

#include "view/viewables/all_viewables_declaration.h"

#include "view/viewables/particle_types.h"

#include "view/audiovisual_state/audiovisual_profiler.h"
#include "view/audiovisual_state/aabb_highlighter.h"
#include "view/game_gui/elements/character_gui.h"
#include "view/game_gui/elements/item_button.h"
#include "view/game_gui/elements/slot_button.h"

#include "augs/audio/audio_settings.h"

#include "view/audiovisual_state/systems/interpolation_settings.h"

#include "view/audiovisual_state/all_audiovisual_systems.h"
#include "view/audiovisual_state/systems/randomizing_system.h"
#include "view/audiovisual_state/audiovisual_post_solve_settings.h"
#include "view/audiovisual_state/particle_triangle_buffers.h"
#include "application/performance_settings.h"
#include "view/character_camera.h"

class cosmos;
class visible_entities;

namespace augs {
	class thread_pool;
	class audio_command_buffers;
	struct dedicated_buffers;
}

struct character_camera;
struct damage_indication_settings;

struct audiovisual_post_solve_input {
	const augs::audio_renderer* audio_renderer;
	const particle_effects_map& particle_effects;
	const loaded_sounds_map& sounds;
	const augs::audio_volume_settings audio_volume;
	const sound_system_settings& sound_settings;
	const character_camera& camera;
	const performance_settings& performance;
	const damage_indication_settings& damage_indication;
	const audiovisual_post_solve_settings settings;
};

struct audiovisual_advance_input {
	augs::audio_command_buffers& command_buffers;
	const augs::audio_renderer* audio_renderer;
	const augs::delta frame_delta;
	const double speed_multiplier;
	const double inv_tickrate;
	const double interpolation_ratio;

	const character_camera camera;
	const camera_cone queried_cone;

	const visible_entities& all_visible;
	const particle_effects_map& particle_effects;
	const plain_animations_pool& plain_animations;

	const loaded_sounds_map& sounds;
	const augs::audio_volume_settings& audio_volume;
	const sound_system_settings& sound_settings;
	const performance_settings& performance;
	const images_in_atlas_map& game_images;
	particle_triangle_buffers& particles_output;
	augs::dedicated_buffers& dedicated;
	const std::optional<augs::delta> new_state_delta;

	const damage_indication_settings& damage_indication;

	augs::thread_pool& pool;
};

struct audiovisual_state {
	aabb_highlighter world_hover_highlighter;
	all_audiovisual_systems systems;

	audiovisual_profiler performance;

	randomizing_system randomizing;
	
	template <class T>
	auto& get() {
		return systems.get<T>();
	}

	template <class T>
	const auto& get() const {
		return systems.get<T>();
	}

	void advance(const audiovisual_advance_input);
	void standard_post_solve(const const_logic_step, audiovisual_post_solve_input);
	void spread_past_infection(const const_logic_step);
	void reserve_caches_for_entities(const std::size_t);

private:
	randomization& get_rng() {
		return randomizing.rng;
	}
};