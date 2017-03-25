#pragma once
#include "augs/misc/pooled_object_id.h"
#include "augs/misc/trivially_copyable_tuple.h"

namespace augs {
	template <class... components>
	class component_aggregate {
	public:
		typedef trivially_copyable_tuple<pooled_object_id<components>...> component_id_tuple;

		component_id_tuple component_ids;

		template <class component>
		auto& writable_id() {
			return std::get<pooled_object_id<component>>(component_ids);
		}

		template <class component>
		auto get_id() const {
			return std::get<pooled_object_id<component>>(component_ids);
		}
	};
}
