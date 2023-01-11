#pragma once

void create_lights(const intercosm& scene, editor_resource_pools& pools) {
	auto& pool = pools.template get_pool_for<editor_light_resource>();

	{
		using test_id_type = test_static_lights;

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto light = scene.world.get_flavour(to_entity_flavour_id(enum_id)).template get<components::light>();

			auto res = editor_light_resource();
			res.editable = light;
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
			res.official_tag = enum_id;

			pool.allocate(res);
		});
	}

#if CREATE_OFFICIAL_CONTENT_ON_EDITOR_LEVEL
	{
		auto& strong_lamp = create_official(official_lights::STRONG_LAMP, pools);
		(void)strong_lamp;
	}

	{
		auto& aquarium_lamp = create_official(official_lights::AQUARIUM_LAMP, pools).editable;
		aquarium_lamp.attenuation.constant = 75;
		aquarium_lamp.attenuation.quadratic = 631;
	}
#endif
}

