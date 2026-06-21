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
			const auto& cp_def = scene.world.get_flavour(flavour_id).template get<invariants::continuous_particles>();
			const auto& cp = scene.world.get_flavour(flavour_id).template get<components::continuous_particles>();
			//const auto effect = .effect.modifier;

			auto res = editor_particles_resource();
			//static_cast<particle_effect_modifier&>(res.editable) = effect;
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;
			res.scene_asset_id = cp_def.effect_id;
			res.editable.wandering = cp_def.wandering;
			res.editable.color = cp.modifier.color;

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
}
