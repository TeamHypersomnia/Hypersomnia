#pragma once

template <class A, class B>
auto float_range(const A a, const B b) {
	return augs::bound<float>(static_cast<float>(a), static_cast<float>(b));
}

void create_particles(const intercosm& scene, editor_resource_pools& pools) {
	(void)scene;

	{
		auto& pool = pools.template get_pool_for<editor_particles_resource>();
		using test_id_type = test_particles_decorations;

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);
			const auto& cp = scene.world.get_flavour(flavour_id).template get<invariants::continuous_particles>();
			//const auto effect = .effect.modifier;

			auto res = editor_particles_resource();
			//static_cast<particle_effect_modifier&>(res.editable) = effect;
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;
			res.editable.wandering = cp.wandering;
			res.editable.color = cp.effect.modifier.color;

			pool.allocate(res);
		});
	}

	{
		auto& pool = pools.template get_pool_for<editor_wandering_pixels_resource>();
		using test_id_type = test_wandering_pixels_decorations;

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);
			//const auto& wp =       scene.world.get_flavour(flavour_id).template get<invariants::wandering_pixels>();
			auto& flavour = scene.world.get_flavour(flavour_id);
			const auto& def_comp = flavour.template get<components::wandering_pixels>();
			const auto& og = flavour.template get<components::overridden_geo>();

			auto res = editor_wandering_pixels_resource();
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;
			res.editable.node_defaults = def_comp;
			res.editable.default_size = og.size.value;

			pool.allocate(res);
		});
	}

#if CREATE_OFFICIAL_CONTENT_ON_EDITOR_LEVEL
	auto create_particles = [&](const official_particles id) -> auto& {
		return create_official(id, pools).editable;
	};

	auto default_bounds = [](particles_emission& em) {
		em.swings_per_sec_bound = { { 0.15f, 0.25f },{ 0.30f, 0.50f } };
		em.swing_spread_bound = { { 0.5f, 1.0f },{ 5.0f, 6.0f } };
	};

	{
		// TODO this is just a test, will need proper defs
		auto& effect = create_particles(official_particles::GLASS_DAMAGE);

		{
			particles_emission em;
			em.rotation_speed = float_range(0, 0);

			effect.emissions.push_back(em);
		}

		{
			particles_emission em;
			default_bounds(em);

			em.randomize_spawn_point_within_circle_of_inner_radius = float_range(13.f, 13.f);
			em.randomize_spawn_point_within_circle_of_outer_radius = float_range(20.f, 20.f);

			em.target_layer = particle_layer::DIM_SMOKES;
			em.initial_rotation_variation = 180;
			em.randomize_acceleration = false;

			effect.emissions.push_back(em);
		}
	}

#endif
}
