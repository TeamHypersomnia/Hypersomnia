#include "application/setups/editor/packaged_official_content.h"
#include "application/setups/editor/editor_official_resource_map.hpp"
#include "application/setups/editor/defaults/editor_resource_defaults.h"

template <class R, class F>
void packaged_official_content::for_each_resource(F callback) {
	resources.template get_pool_for<R>().for_each_id_and_object(
		[&](const auto& raw_id, const auto& object) {
			const bool official = true;
			const auto typed_id = editor_typed_resource_id<R>::from_raw(raw_id, official);

			callback(typed_id, object);
		}
	);
}

void create_official_resources(
	const intercosm& initial_intercosm,
	editor_resource_pools& pools
);

packaged_official_content::packaged_official_content(sol::state& lua) {
	built_content.populate_official_content(
		lua,
		60
	);

	::create_official_resources(built_content, resources);

	auto map_with_tag = [&](const auto id, auto& obj) {
		ensure(obj.official_tag.has_value());

		if (obj.official_tag) {
			resource_map[*obj.official_tag] = id;
		}
	};

	auto map_with_type = [&](const auto id, auto& obj) {
		resource_map[obj.editable.type] = id;
	};

	resources.pools.for_each_container(
		[&]<typename P>(const P&) {
			using R = typename P::mapped_type;

			if constexpr(!is_one_of_v<R, editor_prefab_resource, editor_game_mode_resource>) {
				if constexpr(is_one_of_v<R, editor_area_marker_resource, editor_point_marker_resource>) {
					for_each_resource<R>(map_with_type);
				}
				else {
					for_each_resource<R>(map_with_tag);
				}
			}
		}
	);

	create_official_prefabs();
	for_each_resource<editor_prefab_resource>(map_with_type);

	auto find_lambda = [&](auto id) {
		return resources.find_typed(id);
	};

	::setup_resource_defaults_after_creating_officials(find_lambda, resource_map);
}

const editor_resource_pools& official_get_resources(const packaged_official_content& official) {
	return official.resources;
}

const editor_official_resource_map& official_get_resource_map(const packaged_official_content& official) {
	return official.resource_map;
}

