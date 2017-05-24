#pragma once
#include "augs/templates/for_each_in_types.h"
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/entity_system/component_aggregate.h"

#include "augs/misc/pool.h"
#include "augs/misc/pooled_object_id.h"

namespace augs {
	template <class derived, class... components>
	class operations_on_all_components_mixin {
		typedef component_aggregate<components...> aggregate_type;
		typedef pooled_object_id<aggregate_type> aggregate_id;

	public:
		typedef pool<aggregate_type> aggregate_pool_type;
		typedef std::tuple<pool<components>...> component_pools_type;

		void reserve_storage_for_all_components(const size_t n) {
			auto& self = *static_cast<derived*>(this);

			auto r = [&self, n](auto c) {
				auto& component_pool = self.template get_component_pool<decltype(c)>();
				component_pool.initialize_space(n);
			};

			for_each_type<components...>(r);
		}

		template <class... excluded_components, class handle_type>
		void clone_all_components_except(
			const handle_type into,
			const handle_type from 
		) {
			for_each_type<components...>([&from, &into](auto c) {
				if (is_one_of_v<decltype(c), excluded_components...>) {
					return;
				}

				if (from.allocator::template has<decltype(c)>()) {
					if (into.allocator::template has<decltype(c)>()) {
						into.allocator::template get<decltype(c)>() = from.allocator::template get<decltype(c)>();
					}
					else {
						into.allocator::template add<decltype(c)>(from.allocator::template get<decltype(c)>());
					}
				}
			});
		}

		template <class handle_type>
		void free_all_components(const handle_type handle) {
			auto& self = *static_cast<derived*>(this);

			for_each_type<components...>([&](auto c) {
				typedef decltype(c) component;

				if(handle.allocator::template has<component>()) {
					self.template get_component_pool<component>().free(handle.get().template get_id<component>());
				}
			});
		}
	};
}
