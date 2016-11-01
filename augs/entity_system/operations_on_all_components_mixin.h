#pragma once
#include "augs/templates/tuple_of.h"
#include "augs/templates/for_each_in_types.h"

#include "augs/misc/pool.h"
#include "augs/misc/pool_id.h"

namespace augs {
	template <class derived, class... components>
	class operations_on_all_components_mixin {
		typedef component_aggregate<components...> aggregate_type;
		typedef pool_id<aggregate_type> aggregate_id;

	public:
		typedef pool<aggregate_type> aggregate_pool_type;
		typedef tuple_of_t<make_pool, components...> component_pools_type;

		void reserve_storage_for_all_components(size_t n) {
			auto& self = *static_cast<derived*>(this);

			auto r = [&self, n](auto c) {
				auto& component_pool = self.get_component_pool<decltype(c)>();
				component_pool.initialize_space(n);
			};

			for_each_type<components...>(r);
		}

		template <class excluded_component, class handle_type>
		void clone_all_components(handle_type from, handle_type into) {
			for_each_type<components...>([&from, &into](auto c) {
				if (std::is_same<excluded_component, decltype(c)>::value)
					return;

				if (from.template has<decltype(c)>())
					into += from.template get<decltype(c)>();
			});
		}

		template <class handle_type>
		void remove_all_components(handle_type handle) {
			for_each_type<components...>([&handle](auto c) {
				if(handle.template has<decltype(c)>())
					handle.template remove<decltype(c)>();
			});
		}
	};
}
