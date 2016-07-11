#pragma once
#include "templates.h"
#include "misc/object_pool_id.h"

namespace augs {
	template <class... components>
	class component_aggregate {
	public:
		typename transform_types<std::tuple, make_object_pool_id, components...>::type component_ids;
		object_pool_id<component_aggregate> this_id;

		template <class component>
		auto& writable_id() {
			return std::get<object_pool_id<component>>(component_ids);
		}

		template <class component>
		auto get_id() const {
			return std::get<object_pool_id<component>>(component_ids);
		}
	};
}
