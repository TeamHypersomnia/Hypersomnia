#pragma once
#include "augs/entity_system/storage_for_systems.h"

#include "game/transcendental/types_specification/all_systems_audiovisual_includes.h"
#include "game/transcendental/types_specification/all_systems_declaration.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_id.h"

#include "game/assets/assets_declarations.h"

#include "game/detail/particle_types.h"

#include "game/detail/gui/aabb_highlighter.h"
#include "game/detail/gui/character_gui.h"
#include "game/detail/gui/item_button.h"
#include "game/detail/gui/slot_button.h"
#include "game/view/world_camera.h"

#include "game/view/world_camera.h"
#include "augs/audio/audio_settings.h"
#include "game/systems_audiovisual/interpolation_settings.h"
#include "game/transcendental/types_specification/all_component_includes.h"

class cosmos;
struct visible_entities;

struct audiovisual_advance_input {
	const cosmos& cosm;
	const entity_id viewed_character_id;
	const visible_entities& all_visible;
	const float speed_multiplier;
	const vec2i screen_size;
	const particle_effect_definitions& particle_effects;

	const loaded_sounds& sounds;
	const game_gui_context_dependencies gui_deps;
	const augs::audio_volume_settings audio_volume;
	const interpolation_settings interpolation;
	const world_camera_settings camera;
};

struct audiovisual_state {
	world_camera camera;
	aabb_highlighter world_hover_highlighter;
	storage_for_all_systems_audiovisual systems;

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