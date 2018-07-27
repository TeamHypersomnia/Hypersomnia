#pragma once
#include "augs/entity_system/storage_for_systems.h"
#include "augs/math/camera_cone.h"

#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_id.h"

#include "view/viewables/all_viewables_declarations.h"

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

class cosmos;
class visible_entities;

struct character_camera;

struct audiovisual_post_solve_input {
	const particle_effects_map& particle_effects;
	const loaded_sounds_map& sounds;
	const augs::audio_volume_settings audio_volume;
	const character_camera& camera;
};

struct audiovisual_advance_input {
	const augs::delta frame_delta;
	const double speed_multiplier;

	const character_camera& camera;

	const visible_entities& all_visible;
	const particle_effects_map& particle_effects;
	const plain_animations_pool& plain_animations;

	const loaded_sounds_map& sounds;
	const augs::audio_volume_settings audio_volume;
};

struct audiovisual_state {
	aabb_highlighter world_hover_highlighter;
	all_audiovisual_systems systems;

	audiovisual_profiler performance;

	mutable randomizing_system randomizing;
	
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
	void standard_post_cleanup(const const_logic_step);

	void spread_past_infection(const const_logic_step);

	void reserve_caches_for_entities(const std::size_t);

	void clear();
	void clear_dead_entities(const cosmos&);

private:
	randomization& get_rng() const {
		return randomizing.rng;
	}
};