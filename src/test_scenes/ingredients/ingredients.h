#pragma once
#if BUILD_TEST_SCENES
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"

#include "game/assets/ids/image_id.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/render_component.h"
#include "game/components/sprite_component.h"

#include "test_scenes/test_scene_images.h"
#include "test_scenes/test_scene_flavours.h"

#include "test_scenes/ingredients/box_physics.h"
#include "test_scenes/ingredients/sprite.h"

namespace ingredients {
	void add_standard_pathfinding_capability(entity_handle);
	void add_soldier_intelligence(entity_handle);
}

namespace prefabs {
	entity_handle create_car(const logic_step, const components::transform&);

	// guns
	entity_handle create_sample_magazine(const logic_step, components::transform pos, entity_id charge_inside = entity_id(), int force_num_charges = -1);
	entity_handle create_sample_rifle(const logic_step, vec2 pos, entity_id load_mag = entity_id());
	entity_handle create_kek9(const logic_step step, vec2 pos, entity_id load_mag_id);
	entity_handle create_amplifier_arm(
		const logic_step,
		const vec2 pos 
	);
	entity_handle create_cyan_charge(const logic_step, vec2 pos);

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