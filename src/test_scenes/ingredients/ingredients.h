#pragma once
#if BUILD_TEST_SCENES
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"

#include "game/assets/ids/asset_ids.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/render_component.h"
#include "game/components/sprite_component.h"

#include "test_scenes/test_scene_images.h"
#include "test_scenes/test_scene_sounds.h"
#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/test_scene_physical_materials.h"
#include "test_scenes/test_scene_particle_effects.h"
#include "test_scenes/test_scene_recoil_players.h"

#include "test_scenes/ingredients/box_physics.h"
#include "test_scenes/ingredients/sprite.h"

#include "view/viewables/image_cache.h"

template <class E>
void add_shape_invariant_from_renderable(
	E& into,
	const loaded_image_caches_map& caches
) {
	static_assert(E::template has<invariants::shape_polygon>());

	if (const auto sprite = into.template find<invariants::sprite>()) {
		const auto image_size = caches.at(sprite->image_id).get_original_size();
		vec2 scale = sprite->get_size() / image_size;

		invariants::shape_polygon shape_polygon_def;

		shape_polygon_def.shape = caches.at(sprite->image_id).make_box();
		shape_polygon_def.shape.scale(scale);

		into.template set(shape_polygon_def);
	}

	if (const auto polygon = into.template find<invariants::polygon>()) {
		std::vector<vec2> input;

		input.reserve(polygon->vertices.size());

		for (const auto& v : polygon->vertices) {
			input.push_back(v.pos);
		}

		invariants::shape_polygon shape_polygon_def;
		shape_polygon_def.shape.add_concave_polygon(input);

		into.template set(shape_polygon_def);
	}
}

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

	entity_handle create_sample_backpack(const logic_step, vec2 pos);
	entity_handle create_brown_backpack(const logic_step, vec2 pos);

	entity_handle create_metropolis_soldier(
		const logic_step,
		const transformr pos, 
		const std::string name = "character_unnamed"
	);

	entity_handle create_resistance_soldier(
		const logic_step,
		const transformr pos, 
		const std::string name = "character_unnamed"
	);

	entity_handle create_crate(const logic_step, const transformr pos);
	entity_handle create_brick_wall(const logic_step, const transformr pos);

	entity_handle create_cyan_urban_machete(const logic_step, const vec2 pos);

	entity_handle create_force_grenade(const logic_step, const vec2 pos);
	entity_handle create_ped_grenade(const logic_step, const vec2 pos);
	entity_handle create_interference_grenade(const logic_step, const vec2 pos);

	entity_handle create_rotating_fan(const logic_step, const transformr&);
	entity_handle create_yellow_fish(const logic_step, const transformr&, const transformr& origin, unsigned frame_offset = 0);

	entity_handle create_jellyfish(const logic_step, const transformr&, const transformr& origin, unsigned frame_offset = 0);
}
#endif