
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

	augs::for_each_enum_except_bounds([&](const test_id_type) {
		all_definitions.allocate();
	});

	using bound = augs::bound<real32>;

	const auto set_default = [&](
		const test_scene_physical_material_id a,
		const test_scene_sound_id c,
		std::optional<collision_sound_def> maybe_def_template = std::nullopt
	) {
		const auto a_id = to_physical_material_id(a);

		collision_sound_def def_template;

		if (maybe_def_template.has_value()) {
			def_template = *maybe_def_template;
		}
		else {
			def_template.mute_after_playing_times = 3;
		}

		const auto c_id = to_sound_id(c);
		def_template.effect.id = c_id;
		all_definitions[a_id].default_collision = def_template;
	};
	(void)set_default;

	const auto set_pair = [&](
		const test_scene_physical_material_id a,
		const test_scene_physical_material_id b,
		const test_scene_sound_id c,
		const bool silence_other = true,
		std::optional<collision_sound_def> maybe_def_template = std::nullopt
	) {
		const auto a_id = to_physical_material_id(a);
		const auto b_id = to_physical_material_id(b);
		const auto c_id = to_sound_id(c);

		collision_sound_def def_template;

		if (maybe_def_template.has_value()) {
			def_template = *maybe_def_template;
		}

		{
			auto& entry = all_definitions[a_id].collision_sound_matrix[b_id];
			def_template.effect.id = c_id;
			entry = def_template;

			if (silence_other) {
				entry.silence_opposite_collision_sound = true;
			}
		}
	};

	{
		collision_sound_def def_template;
		def_template.silence_opposite_collision_sound = true;

		set_default(test_scene_physical_material_id::CHARACTER, test_scene_sound_id::BLANK, def_template);
	}

	set_default(test_scene_physical_material_id::WOOD, test_scene_sound_id::COLLISION_METAL_WOOD);
	set_default(test_scene_physical_material_id::GLASS, test_scene_sound_id::COLLISION_GLASS);
	set_default(test_scene_physical_material_id::FLASHBANG, test_scene_sound_id::COLLISION_FLASHBANG);
	set_default(test_scene_physical_material_id::GRENADE, test_scene_sound_id::COLLISION_GRENADE);
	set_default(test_scene_physical_material_id::METAL, test_scene_sound_id::COLLISION_METAL_METAL);

	{
		collision_sound_def def;
		def.pitch_bound = bound(0.9f, 1.25f);
		def.collision_sound_sensitivity = 5.0f;

		def.mute_after_playing_times = 1;

		set_pair(test_scene_physical_material_id::KNIFE, test_scene_physical_material_id::WOOD, test_scene_sound_id::COLLISION_KNIFE_WOOD, true, def);
		set_default(test_scene_physical_material_id::KNIFE, test_scene_sound_id::COLLISION_KNIFE_METAL, def);
	}

	{
		collision_sound_def def;
		def.mute_after_playing_times = 1;
		def.effect.modifier.gain *= 0.6f;

		set_default(test_scene_physical_material_id::VENT, test_scene_sound_id::AIR_DUCT_COLLISION, def);

		set_pair(test_scene_physical_material_id::VENT, test_scene_physical_material_id::WOOD, test_scene_sound_id::AIR_DUCT_COLLISION, true, def);
		set_pair(test_scene_physical_material_id::VENT, test_scene_physical_material_id::METAL, test_scene_sound_id::AIR_DUCT_COLLISION, true, def);
	}

	set_pair(test_scene_physical_material_id::METAL, test_scene_physical_material_id::WOOD, test_scene_sound_id::COLLISION_METAL_WOOD, true);

	{
		auto& metal = all_definitions[to_physical_material_id(test_scene_physical_material_id::METAL)];
		metal.standard_damage_sound.id = to_sound_id(test_scene_sound_id::COLLISION_METAL_METAL);
		metal.standard_damage_particles.id = to_particle_effect_id(test_scene_particle_effect_id::METAL_DAMAGE);
		metal.standard_damage_particles.modifier.color = rgba(251, 255, 181, 255);
		metal.unit_damage_for_effects = 20.f;
	}

	{
		auto& vent = all_definitions[to_physical_material_id(test_scene_physical_material_id::VENT)];
		vent.standard_damage_sound.id = to_sound_id(test_scene_sound_id::AIR_DUCT_IMPACT);
		vent.standard_damage_sound.modifier.pitch = 1.f;
		vent.standard_damage_particles.id = to_particle_effect_id(test_scene_particle_effect_id::METAL_DAMAGE);
		vent.standard_damage_particles.modifier.color = rgba(251, 255, 181, 255);
		vent.unit_damage_for_effects = 20.f;
	}

	{
		auto& wood = all_definitions[to_physical_material_id(test_scene_physical_material_id::WOOD)];
		wood.standard_damage_sound.id = to_sound_id(test_scene_sound_id::WOOD_DAMAGE);
		wood.standard_damage_particles.id = to_particle_effect_id(test_scene_particle_effect_id::WOOD_DAMAGE);
		wood.standard_damage_particles.modifier.color = rgba(204 - 50, 182 - 50, 175 - 50, 255);
	}

	{
		auto& glass = all_definitions[to_physical_material_id(test_scene_physical_material_id::GLASS)];
		glass.standard_damage_sound.id = to_sound_id(test_scene_sound_id::GLASS_DAMAGE);
		glass.standard_damage_particles.id = to_particle_effect_id(test_scene_particle_effect_id::GLASS_DAMAGE);
		glass.standard_damage_particles.modifier.color = rgba(142, 186, 197, 255);
		glass.silence_damager_destruction_sound = true;
	}
}