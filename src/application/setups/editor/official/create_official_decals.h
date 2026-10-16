#pragma once

void create_decals(const intercosm& scene, editor_resource_pools& pools) {
	(void)scene;

	auto& pool = pools.template get_pool_for<editor_decal_resource>();
	using test_id_type = test_decal_decorations;

	augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
		auto res = editor_decal_resource();
		res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
		res.official_tag = enum_id;
		res.scene_flavour_id = to_entity_flavour_id(enum_id);

		pool.allocate(res);
	});
}
