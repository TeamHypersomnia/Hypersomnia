#include "augs/templates/enum_introspect.h"
#include "test_scenes/test_scene_physical_materials.h"
#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_sounds.h"
#include "augs/misc/pool/pool_allocate.h"

#include "game/assets/physical_material.h"
#include "game/assets/all_logical_assets.h"
#include "augs/string/format_enum.h"

#include "test_scenes/test_scene_particle_effects.h"

void load_test_scene_physical_materials(physical_materials_pool& all_definitions) {
	using test_id_type = test_scene_physical_material_id;

	all_definitions.reserve(enum_count(test_id_type()));

	augs::for_each_enum_except_bounds([&](const test_id_type id) {
		all_definitions.allocate().object.name = format_enum(id);
	});

	const auto set_pair = [&](
		const test_scene_physical_material_id a,
		const test_scene_physical_material_id b,
		const test_scene_sound_id c,
		const bool both_ways = true
	) {
		const auto a_id = to_physical_material_id(a);
		const auto b_id = to_physical_material_id(b);
		const auto c_id = to_sound_id(c);

		all_definitions[a_id].collision_sound_matrix[b_id] = c_id;

		if (both_ways) {
			all_definitions[b_id].collision_sound_matrix[a_id] = c_id;
		}
	};

	set_pair(test_scene_physical_material_id::KNIFE, test_scene_physical_material_id::METAL, test_scene_sound_id::COLLISION_KNIFE_METAL);
	set_pair(test_scene_physical_material_id::KNIFE, test_scene_physical_material_id::WOOD, test_scene_sound_id::COLLISION_KNIFE_WOOD);
	set_pair(test_scene_physical_material_id::KNIFE, test_scene_physical_material_id::GLASS, test_scene_sound_id::COLLISION_GLASS);
	set_pair(test_scene_physical_material_id::KNIFE, test_scene_physical_material_id::GRENADE, test_scene_sound_id::COLLISION_GRENADE);

	set_pair(test_scene_physical_material_id::METAL, test_scene_physical_material_id::METAL, test_scene_sound_id::COLLISION_METAL_METAL);
	set_pair(test_scene_physical_material_id::METAL, test_scene_physical_material_id::WOOD, test_scene_sound_id::COLLISION_METAL_WOOD);
	set_pair(test_scene_physical_material_id::WOOD, test_scene_physical_material_id::WOOD, test_scene_sound_id::COLLISION_METAL_WOOD);

	set_pair(test_scene_physical_material_id::GRENADE, test_scene_physical_material_id::WOOD, test_scene_sound_id::COLLISION_GRENADE);
	set_pair(test_scene_physical_material_id::GRENADE, test_scene_physical_material_id::METAL, test_scene_sound_id::COLLISION_GRENADE);
	set_pair(test_scene_physical_material_id::GRENADE, test_scene_physical_material_id::GRENADE, test_scene_sound_id::COLLISION_GRENADE);

	set_pair(test_scene_physical_material_id::GLASS, test_scene_physical_material_id::METAL, test_scene_sound_id::COLLISION_GLASS, false);
	set_pair(test_scene_physical_material_id::METAL, test_scene_physical_material_id::GLASS, test_scene_sound_id::COLLISION_METAL_METAL, false);
	set_pair(test_scene_physical_material_id::GLASS, test_scene_physical_material_id::WOOD, test_scene_sound_id::COLLISION_GLASS);

	set_pair(test_scene_physical_material_id::GRENADE, test_scene_physical_material_id::GLASS, test_scene_sound_id::COLLISION_GRENADE, false);
	set_pair(test_scene_physical_material_id::GLASS, test_scene_physical_material_id::GRENADE, test_scene_sound_id::COLLISION_GLASS, false);

	set_pair(test_scene_physical_material_id::GLASS, test_scene_physical_material_id::GLASS, test_scene_sound_id::COLLISION_GLASS);

	{
		auto& metal = all_definitions[to_physical_material_id(test_scene_physical_material_id::METAL)];
		metal.standard_damage_sound.id = to_sound_id(test_scene_sound_id::COLLISION_METAL_METAL);
		metal.standard_damage_particles.id = to_particle_effect_id(test_scene_particle_effect_id::METAL_DAMAGE);
		metal.standard_damage_particles.modifier.colorize = rgba(251, 255, 181, 255);
		metal.unit_effect_damage = 20.f;
	}

	{
		auto& wood = all_definitions[to_physical_material_id(test_scene_physical_material_id::WOOD)];
		wood.standard_damage_sound.id = to_sound_id(test_scene_sound_id::WOOD_DAMAGE);
		wood.standard_damage_particles.id = to_particle_effect_id(test_scene_particle_effect_id::WOOD_DAMAGE);
		wood.standard_damage_particles.modifier.colorize = rgba(204 - 50, 182 - 50, 175 - 50, 255);
	}

	{
		auto& glass = all_definitions[to_physical_material_id(test_scene_physical_material_id::GLASS)];
		glass.standard_damage_sound.id = to_sound_id(test_scene_sound_id::GLASS_DAMAGE);
		glass.standard_damage_particles.id = to_particle_effect_id(test_scene_particle_effect_id::GLASS_DAMAGE);
		glass.standard_damage_particles.modifier.colorize = rgba(142, 186, 197, 255);
	}
}