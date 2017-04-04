#pragma once
#include "game/transcendental/entity_handle.h"
#include "augs/math/vec2.h"

#include "game/assets/game_image_id.h"
#include "game/components/render_component.h"

#include "augs/graphics/pixel.h"
#include "game/transcendental/entity_handle.h"

class cosmos;
#include "game/transcendental/step_declaration.h"

namespace components {
	struct item;
	struct sprite;
}

namespace ingredients {
	void add_camera(entity_handle, int w, int h);

	components::item& make_item(entity_handle);
	void make_always_visible(entity_handle);

	components::sprite& add_sprite(entity_handle, components::transform pos, assets::game_image_id = assets::game_image_id::BLANK, rgba col = rgba(255, 255, 255, 255), render_layer = render_layer::GROUND);
	components::sprite& add_sprite_scalled(entity_handle, components::transform pos, vec2i size = vec2i(), assets::game_image_id = assets::game_image_id::BLANK, rgba col = rgba(255, 255, 255, 255), render_layer = render_layer::GROUND);
	
	void add_bullet_round_physics(entity_handle);
	void add_see_through_dynamic_body(entity_handle);
	void add_shell_dynamic_body(entity_handle);
	void add_standard_dynamic_body(entity_handle, const bool destructible = false);
	void add_standard_static_body(entity_handle);

	void add_wsad_character_physics(entity_handle);
	void add_wsad_character_legs(entity_handle legs, entity_handle player);
	void add_wsad_character(entity_handle, entity_handle crosshair_entity);

	void add_character_inventory(entity_handle);
	void add_backpack(entity_handle);

	void add_default_gun_container(entity_handle, const float mag_rotation = -90.f);

	void add_standard_pathfinding_capability(entity_handle);
	void add_soldier_intelligence(entity_handle);
}

namespace prefabs {
	entity_handle create_car(cosmos&, const components::transform&);
	entity_handle create_motorcycle(cosmos&, const components::transform&);

	entity_handle create_sample_suppressor(cosmos& world, vec2 pos);

	entity_handle create_sample_magazine(const logic_step, components::transform pos, std::string space = "0.30", entity_id charge_inside = entity_id());
	entity_handle create_small_magazine(const logic_step, components::transform pos, std::string space = "0.30", entity_id charge_inside = entity_id());
	entity_handle create_sample_rifle(const logic_step, vec2 pos, entity_id load_mag = entity_id());
	entity_handle create_sample_bilmer2000(const logic_step, vec2 pos, entity_id load_mag = entity_id());
	entity_handle create_pistol(const logic_step, vec2 pos, entity_id load_mag = entity_id());
	entity_handle create_kek9(const logic_step, vec2 pos, entity_id load_mag = entity_id());
	entity_handle create_submachine(const logic_step, vec2 pos, entity_id load_mag = entity_id());

	entity_handle create_amplifier_arm(
		cosmos&, 
		vec2 pos 
	);

	entity_handle create_red_charge(cosmos&, vec2 pos, int charges = 23);
	entity_handle create_pink_charge(cosmos&, vec2 pos, int charges = 23);
	entity_handle create_cyan_charge(cosmos&, vec2 pos, int charges = 30);
	entity_handle create_green_charge(cosmos& world, vec2 pos, int charges = 23);

	entity_handle create_sample_backpack(cosmos&, vec2 pos);

	entity_handle create_character_crosshair(cosmos&);
	
	entity_handle create_sample_complete_character(
		cosmos&, 
		const components::transform pos, 
		const std::string name = "character_unnamed"
	);
	
	entity_handle create_crate(cosmos&, const components::transform pos, const vec2 size = vec2());
	entity_handle create_brick_wall(cosmos&, const components::transform pos, const vec2 size = vec2());

	entity_handle create_cyan_urban_machete(cosmos&, const vec2 pos);

	entity_handle create_force_grenade(cosmos&, const vec2 pos);
	entity_handle create_ped_grenade(cosmos&, const vec2 pos);
	entity_handle create_interference_grenade(cosmos&, const vec2 pos);
}