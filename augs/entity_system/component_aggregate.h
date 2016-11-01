#pragma once
#include "augs/templates/tuple_of.h"
#include "augs/misc/pool_id.h"

namespace augs {
	template <class... components>
	class component_aggregate {
	public:
		typedef tuple_of_t<make_pool_id, components...> component_id_tuple;

		component_id_tuple component_ids;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(component_ids)
			);
		}

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
