#pragma once
#if BUILD_TEST_SCENES
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"

#include "game/assets/ids/game_image_id.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/render_component.h"
#include "game/components/sprite_component.h"

#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/ingredients/box_physics.h"
#include "test_scenes/ingredients/sprite.h"

namespace components {
	struct item;
}

namespace ingredients {
	void add_character(const logic_step, entity_handle);

	void add_character_head_inventory(const logic_step, entity_handle);

	void add_standard_pathfinding_capability(entity_handle);
	void add_soldier_intelligence(entity_handle);
}

namespace test_flavours {
	void add_bullet_round_physics(entity_flavour& meta);
	void add_see_through_dynamic_body(entity_flavour& meta);
	void add_shell_dynamic_body(entity_flavour& meta);
	void add_standard_dynamic_body(entity_flavour& meta);
	void add_standard_static_body(entity_flavour& meta);

	void add_sprite(
		entity_flavour& t, 
		const loaded_game_image_caches& logicals,
		assets::game_image_id = assets::game_image_id::INVALID,
		rgba col = rgba(255, 255, 255, 255),
		invariants::sprite::special_effect = {}
	);
}

namespace prefabs {
	entity_handle create_car(const logic_step, const components::transform&);

	// guns
	entity_handle create_sample_magazine(const logic_step, components::transform pos, entity_id charge_inside = entity_id());
	entity_handle create_sample_rifle(const logic_step, vec2 pos, entity_id load_mag = entity_id());
	entity_handle create_kek9(const logic_step step, vec2 pos, entity_id load_mag_id);
	entity_handle create_amplifier_arm(
		const logic_step,
		const vec2 pos 
	);
	entity_handle create_cyan_charge(const logic_step, vec2 pos, int charges = 30);

	entity_handle create_sample_backpack(const logic_step, vec2 pos);
	
	entity_handle create_sample_complete_character(
		const logic_step,
		const components::transform pos, 
		const std::string name = "character_unnamed",
		const int create_arm_count = 2
	);

	entity_handle create_crate(const logic_step, const components::transform pos);
	entity_handle create_brick_wall(const logic_step, const components::transform pos);

	entity_handle create_cyan_urban_machete(const logic_step, const vec2 pos);

	entity_handle create_force_grenade(const logic_step, const vec2 pos);
	entity_handle create_ped_grenade(const logic_step, const vec2 pos);
	entity_handle create_interference_grenade(const logic_step, const vec2 pos);
}
#endif