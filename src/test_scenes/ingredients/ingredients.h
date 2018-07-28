#pragma once
#if BUILD_TEST_SCENES
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"

#include "game/assets/ids/asset_ids.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_handle.h"

#include "game/components/render_component.h"
#include "game/components/sprite_component.h"

#include "test_scenes/test_scene_images.h"
#include "test_scenes/test_scene_sounds.h"
#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/test_scene_physical_materials.h"
#include "test_scenes/test_scene_particle_effects.h"
#include "test_scenes/test_scene_recoil_players.h"

#include "test_scenes/ingredients/box_physics.h"
#include "test_scenes/ingredients/add_sprite.h"

#include "view/viewables/image_cache.h"

namespace ingredients {
	void add_standard_pathfinding_capability(entity_handle);
	void add_soldier_intelligence(entity_handle);
}

namespace prefabs {
	entity_handle create_car(const logic_step, const transformr&);

	// guns
	entity_handle create_magazine(const logic_step, transformr pos, test_container_items, entity_id charge_inside = entity_id(), int force_num_charges = -1);

	entity_handle create_sample_magazine(const logic_step, transformr pos, entity_id charge_inside = entity_id(), int force_num_charges = -1);
	entity_handle create_sample_rifle(const logic_step, vec2 pos, entity_id load_mag = entity_id());
	entity_handle create_rifle(const logic_step, vec2 pos, test_shootable_weapons, entity_id load_mag = entity_id());
	entity_handle create_vindicator(const logic_step, vec2 pos, entity_id load_mag = entity_id());
	entity_handle create_kek9(const logic_step step, vec2 pos, entity_id load_mag_id);
	entity_handle create_amplifier_arm(
		const logic_step,
		const vec2 pos 
	);
	entity_handle create_cyan_charge(const logic_step, vec2 pos);
	entity_handle create_steel_charge(const logic_step, vec2 pos);
}
#endif