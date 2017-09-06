#pragma once
#include "augs/entity_system/storage_for_systems.h"

#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_id.h"

#include "view/viewables/all_viewables_declarations.h"

#include "view/viewables/particle_types.h"

#include "view/audiovisual_state/aabb_highlighter.h"
#include "view/game_gui/elements/character_gui.h"
#include "view/game_gui/elements/item_button.h"
#include "view/game_gui/elements/slot_button.h"
#include "view/audiovisual_state/world_camera.h"

#include "view/audiovisual_state/world_camera.h"
#include "augs/audio/audio_settings.h"
#include "view/audiovisual_state/systems/interpolation_settings.h"

#include "view/audiovisual_state/all_audiovisual_systems.h"

class cosmos;
struct visible_entities;

struct audiovisual_advance_input {
	const cosmos& cosmos_to_sample;
	const entity_id viewed_character_id;
	const visible_entities& all_visible;
	const float speed_multiplier;
	const vec2i screen_size;
	const particle_effects_map& particle_effects;

	const loaded_sounds& sounds;
	const augs::audio_volume_settings audio_volume;
	const interpolation_settings interpolation;
	const world_camera_settings camera;
};

struct audiovisual_state {
	world_camera camera;
	aabb_highlighter world_hover_highlighter;
	all_audiovisual_systems systems;

	augs::timer timer;

	template <class T>
	auto& get() {
		return systems.get<T>();
	}

	template <class T>
	const auto& get() const {
		return systems.get<T>();
	}

	auto get_standard_post_solve() {
		return [this](const const_logic_step step) {
			standard_post_solve(step);
		};
	}

	auto get_viewing_camera() const {
		return camera.smoothed_camera;
	}

	void advance(const audiovisual_advance_input);

	void standard_post_solve(const const_logic_step);
	void spread_past_infection(const const_logic_step);

	void reserve_caches_for_entities(const std::size_t);
};