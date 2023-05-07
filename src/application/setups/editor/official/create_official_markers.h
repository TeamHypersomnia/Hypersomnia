#pragma once
#include "test_scenes/test_scene_flavour_ids.h"
#include "test_scenes/test_scene_flavours.h"

void create_markers(const intercosm&, editor_resource_pools& pools) {
	{
		using test_id_type = point_marker_type;

		auto& pool = pools.template get_pool_for<editor_point_marker_resource>();

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			auto res = editor_point_marker_resource();
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
			res.editable.type = enum_id;

			pool.allocate(res);
		});
	}

	{
		using test_id_type = area_marker_type;

		auto& pool = pools.template get_pool_for<editor_area_marker_resource>();

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			if (enum_id == area_marker_type::PREFAB) {
				return;
			}

			auto res = editor_area_marker_resource();
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
			res.editable.type = enum_id;

			pool.allocate(res);
		});
	}
}
