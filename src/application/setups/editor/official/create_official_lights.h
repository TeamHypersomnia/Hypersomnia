#pragma once

void create_lights(const intercosm& scene, editor_resource_pools& pools) {
	(void)scene;

	auto& pool = pools.template get_pool_for<editor_light_resource>();

	{
		using test_id_type = test_static_lights;

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);

			auto res = editor_light_resource();
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;

			pool.allocate(res);
		});
	}

#if CREATE_OFFICIAL_CONTENT_ON_EDITOR_LEVEL
	{
		auto& point_lamp = create_official(official_lights::POINT_LAMP, pools);
		(void)point_lamp;
	}
#endif
}

