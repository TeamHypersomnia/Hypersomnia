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
		void set_id(const pooled_object_id<component> to) {
			std::get<pooled_object_id<component>>(component_ids) = to;
		}

		template <class component>
		auto get_id() const {
			return std::get<pooled_object_id<component>>(component_ids);
		}
	};
}
