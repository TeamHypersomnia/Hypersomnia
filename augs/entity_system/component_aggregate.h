#pragma once
#include "augs/templates.h"
#include "augs/misc/pool_id.h"

namespace augs {
	template <class... components>
	class component_aggregate {
	public:
		tuple_of_t<make_pool_id, components...> component_ids;
		pool_id<component_aggregate> this_id;

		template <class component>
		auto& writable_id() {
			return std::get<pool_id<component>>(component_ids);
		}

		template <class component>
		auto get_id() const {
			return std::get<pool_id<component>>(component_ids);
		}
	};
}
