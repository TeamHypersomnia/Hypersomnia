#pragma once
#include "generated/setting_build_test_scenes.h"

#if BUILD_TEST_SCENES
#include "augs/math/vec2.h"
#include "augs/graphics/pixel.h"


#include "game/assets/game_image_id.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/render_component.h"

class cosmos;

namespace components {
	struct item;
	struct sprite;
}

namespace ingredients {
	components::item& make_item(const entity_handle);
	
	void make_always_visible(entity_handle);

	components::sprite& add_sprite(
		entity_handle, 
		assets::game_image_id = assets::game_image_id::BLANK, 
		rgba col = rgba(255, 255, 255, 255), 
		render_layer = render_layer::GROUND
	);

	components::sprite& add_sprite_scaled(
		entity_handle, 
		vec2i size = vec2i(), 
		assets::game_image_id = assets::game_image_id::BLANK, 
		rgba col = rgba(255, 255, 255, 255), 
		render_layer = render_layer::GROUND
	);
	
	void add_bullet_round_physics(const logic_step, entity_handle, const components::transform);
	void add_see_through_dynamic_body(const logic_step, entity_handle, const components::transform);
	void add_shell_dynamic_body(const logic_step, entity_handle, const components::transform);
	void add_standard_dynamic_body(const logic_step, entity_handle, const components::transform, const bool destructible = false);
	void add_standard_static_body(const logic_step, entity_handle, const components::transform);

	void add_character_head_physics(const logic_step, entity_handle, const components::transform);
	void add_character_legs(const logic_step, entity_handle legs, entity_handle player);
	void add_character(const logic_step, entity_handle, entity_handle crosshair_entity);

	void add_character_head_inventory(const logic_step, entity_handle);
	void add_backpack_container(entity_handle);

	void add_default_gun_container(
		const logic_step, 
		entity_handle, 
		const float mag_rotation = -90.f
	);

	void add_standard_pathfinding_capability(entity_handle);
	void add_soldier_intelligence(entity_handle);
}

namespace prefabs {
	entity_handle create_car(const logic_step, const components::transform&);
	entity_handle create_motorcycle(const logic_step, const components::transform&);

	entity_handle create_sample_suppressor(const logic_step world, const vec2 pos);

	entity_handle create_sample_magazine(const logic_step, components::transform pos, std::string space = "0.30", entity_id charge_inside = entity_id());
	entity_handle create_sample_rifle(const logic_step, vec2 pos, entity_id load_mag = entity_id());

	entity_handle create_amplifier_arm(
		const logic_step,
		const vec2 pos 
	);

	entity_handle create_cyan_charge(const logic_step, vec2 pos, int charges = 30);

	entity_handle create_sample_backpack(const logic_step, vec2 pos);

	entity_handle create_character_crosshair(const logic_step);
	
	entity_handle create_sample_complete_character(
		const logic_step,
		const components::transform pos, 
		const std::string name = "character_unnamed",
		const int create_arm_count = 2
	);
	
	entity_handle create_sample_complete_arm(
		const logic_step,
		const vec2 back_arm_size,
		const vec2 front_arm_size,
		const bool primary = true
	);

	entity_handle create_crate(const logic_step, const components::transform pos, const vec2 size = vec2());
	entity_handle create_brick_wall(const logic_step, const components::transform pos, const vec2 size = vec2());

	entity_handle create_cyan_urban_machete(const logic_step, const vec2 pos);

	entity_handle create_force_grenade(const logic_step, const vec2 pos);
	entity_handle create_ped_grenade(const logic_step, const vec2 pos);
	entity_handle create_interference_grenade(const logic_step, const vec2 pos);

	entity_handle create_electric_missile_def(const logic_step, const components::transform);
}
#endif