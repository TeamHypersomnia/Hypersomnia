#pragma once
#include "augs/templates.h"
#include "augs/misc/pool.h"
#include "augs/misc/pool_id.h"

namespace augs {
	template <class derived, class... components>
	class storage_for_components_and_aggregates {
		typedef component_aggregate<components...> aggregate_type;
		typedef pool_id<aggregate_type> aggregate_id;

	public:
		typedef pool<aggregate_type> aggregate_pool_type;
		typedef tuple_of_t<make_pool, components...> component_pools_type;

		size_t aggregates_count() const {
			const auto& self = *static_cast<const derived*>(this);
			return self.get_pool(aggregate_id()).size();
		}

		void reserve_storage_for_aggregates(size_t n) {
			auto& self = *static_cast<derived*>(this);

			self.get_pool(aggregate_id()).initialize_space(n);

			auto r = [&self, n](auto c) {
				auto& component_pool = self.get_pool(pool_id<decltype(c)>());
				component_pool.initialize_space(n);
			};

			for_each_type<components...>(r);
		}

		aggregate_id allocate_aggregate(std::string debug_name = std::string()) {
			auto& self = *static_cast<derived*>(this);
			aggregate_id new_id = self.get_pool(aggregate_id()).allocate();
			new_id.set_debug_name(debug_name);

			return new_id;
		}

		template <class excluded_component>
		aggregate_id clone_aggregate(aggregate_id cloned_aggregate_id) {
			auto& self = *static_cast<derived*>(this);

			auto cloned_aggregate = self.get_handle(cloned_aggregate_id);

			auto new_aggregate = self.get_handle(self.get_pool(aggregate_id()).allocate());

			for_each_type<components...>([&cloned_aggregate, &new_aggregate](auto c) {
				if (std::is_same<excluded_component, decltype(c)>::value)
					return;

				if (cloned_aggregate.template has<decltype(c)>())
					new_aggregate += cloned_aggregate.template get<decltype(c)>();
			});

			new_aggregate.set_debug_name("+" + cloned_aggregate.get_debug_name());

			return new_aggregate;
		}

		void free_aggregate(aggregate_id aggregate) {
			auto& self = *static_cast<derived*>(this);

			auto handle = self.get_handle(aggregate);

			for_each_type<components...>([&handle](auto c) {
				if(handle.template has<decltype(c)>())
					handle.template remove<decltype(c)>();
			});

			self.get_pool(aggregate_id()).free(aggregate);
		}
	};

}
