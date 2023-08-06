#pragma once
#include "test_scenes/test_scene_flavour_ids.h"
#include "test_scenes/test_scene_flavours.h"

void create_weapons(const intercosm& scene, editor_resource_pools& pools) {
	(void)scene;

	{
		using test_id_type = test_shootable_weapons;

		auto& pool = pools.template get_pool_for<editor_firearm_resource>();

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);

			auto res = editor_firearm_resource();
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;

			pool.allocate(res);
		});
	}

	{
		using test_id_type = test_melee_weapons;

		auto& pool = pools.template get_pool_for<editor_melee_resource>();

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);

			auto res = editor_melee_resource();
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;

			pool.allocate(res);
		});
	}

	{
		using test_id_type = test_hand_explosives;

		auto& pool = pools.template get_pool_for<editor_explosive_resource>();

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);

			auto res = editor_explosive_resource();
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;

			pool.allocate(res);
		});
	}

	{
		using test_id_type = test_shootable_charges;

		auto& pool = pools.template get_pool_for<editor_ammunition_resource>();

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);

			if (enum_id < test_shootable_charges::GRADOBICIE_CHARGE || enum_id > test_shootable_charges::SKULL_ROCKET) {
				return;
			}

			auto res = editor_ammunition_resource();
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;

			pool.allocate(res);
		});
	}

	{
		using test_id_type = test_container_items;

		auto& pool = pools.template get_pool_for<editor_ammunition_resource>();

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);

			auto res = editor_ammunition_resource();
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));

			if (res.unique_name.find("magazine") == std::string::npos) {
				return;
			}

			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;

			pool.allocate(res);
		});
	}

	{
		using test_id_type = test_tool_items;

		auto& pool = pools.template get_pool_for<editor_tool_resource>();

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto flavour_id = to_entity_flavour_id(enum_id);

			auto res = editor_tool_resource();
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
			res.official_tag = enum_id;
			res.scene_flavour_id = flavour_id;

			pool.allocate(res);
		});
	}
}
