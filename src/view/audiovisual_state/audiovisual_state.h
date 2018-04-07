#pragma once
#include "augs/entity_system/storage_for_systems.h"
#include "augs/math/camera_cone.h"

#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_id.h"

#include "view/viewables/all_viewables_declarations.h"

#include "view/viewables/particle_types.h"

#include "view/audiovisual_state/audiovisual_profiler.h"
#include "view/audiovisual_state/aabb_highlighter.h"
#include "view/game_gui/elements/character_gui.h"
#include "view/game_gui/elements/item_button.h"
#include "view/game_gui/elements/slot_button.h"

#include "augs/audio/audio_settings.h"

#include "view/viewer_eye.h"
#include "view/audiovisual_state/systems/interpolation_settings.h"

#include "view/audiovisual_state/all_audiovisual_systems.h"

class cosmos;
struct visible_entities;
class session_profiler;

struct audiovisual_post_solve_input {
	const particle_effects_map& particle_effects;
	const loaded_sounds_map& sounds;

	const viewer_eye eye;
};

struct audiovisual_advance_input {
	const augs::delta frame_delta;
	const double speed_multiplier;

	const viewer_eye eye;

	const visible_entities& all_visible;
	const particle_effects_map& particle_effects;

	const loaded_sounds_map& sounds;
	const augs::audio_volume_settings audio_volume;

	// for now just to know whats going on
	audiovisual_advance_input(
		const augs::delta frame_delta,
		const double speed_multiplier,

		const viewer_eye eye,
		const visible_entities& all_visible,

		const particle_effects_map& particle_effects,

		const loaded_sounds_map& sounds,
		const augs::audio_volume_settings audio_volume
	) :
		frame_delta(frame_delta),
		speed_multiplier(speed_multiplier),

		eye(eye),
		all_visible(all_visible),
		particle_effects(particle_effects),

		sounds(sounds),
		audio_volume(audio_volume)
	{

	}
};

struct audiovisual_state {
	aabb_highlighter world_hover_highlighter;
	all_audiovisual_systems systems;

	audiovisual_profiler profiler;
	
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
};