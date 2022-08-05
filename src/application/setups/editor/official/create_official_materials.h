#pragma once

void create_materials(editor_resource_pools& pools) {
	auto create_material = [&](const official_materials id) -> auto& {
		return create_official(id, pools).editable;
	};

	{
		auto& glass = create_material(official_materials::GLASS);

		glass.standard_damage_sound.resource_id = to_resource_id(official_sounds::GLASS_DAMAGE);

		glass.standard_damage_particles.resource_id = to_resource_id(official_particles::GLASS_DAMAGE);
		glass.standard_damage_particles.colorize = rgba(142, 186, 197, 255);
	}

	//using bound = augs::bound<real32>;

	const auto set_pair = [&](
		const official_materials a,
		const official_materials b,
		const official_sounds c,
		const bool both_ways = true,
		std::optional<editor_collision_sound_def> maybe_def_template = std::nullopt
	) {
		const auto a_id = to_resource_id(a);
		const auto b_id = to_resource_id(b);
		const auto c_id = to_resource_id(c);

		auto def_template = editor_collision_sound_def();

		if (maybe_def_template != std::nullopt) {
			def_template = *maybe_def_template;
		}
		else {
			def_template.occurences_before_cooldown = 3;
		}

		{
			auto& entry = get_resource(a_id, pools).collision_sound_matrix[b_id];
			def_template.effect.resource_id = c_id;
			entry = def_template;
		}

		if (both_ways) {
			auto& entry = get_resource(b_id, pools).collision_sound_matrix[a_id];

			def_template.effect.resource_id = c_id;
			entry = def_template;
		}
	};

	set_pair(official_materials::GLASS, official_materials::GLASS, official_sounds::COLLISION_GLASS);
}
