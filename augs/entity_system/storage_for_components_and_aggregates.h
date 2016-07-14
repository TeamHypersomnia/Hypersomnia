#pragma once
#include "templates.h"
#include "misc/pool.h"
#include "misc/pool_id.h"

#include "aggregate_mixins.h"

namespace augs {
	template <class handle_owner_type, class... components>
	class storage_for_components_and_aggregates {
		typedef component_aggregate<components...> aggregate_type;
		typedef pool_id<aggregate_type> aggregate_id;

	public:
		typedef pool<aggregate_type> aggregate_pool_type;

		aggregate_pool_type pool_for_aggregates;
		tuple_of_t<make_pool, components...> pools_for_components;

		template<class T>
		auto& get_pool(pool_id<T>) {
			return std::get<pool<T>>(pools_for_components);
		}

		template<class T>
		const auto& get_pool(pool_id<T>) const {
			return std::get<pool<T>>(pools_for_components);
		}

		template<>
		auto& get_pool(aggregate_id) {
			return pool_for_aggregates;
		}

		template<>
		const auto& get_pool(aggregate_id) const {
			return pool_for_aggregates;
		}

		template<class component, class... Args>
		auto allocate_component(Args... args) {
			return get_component_pool<component>().allocate(args...);
		}

		size_t aggregates_count() const {
			return pool_for_aggregates.size();
		}

		void reserve_storage_for_aggregates(size_t n) {
			pool_for_aggregates.initialize_space(n);

			auto r = [this, n](auto elem) {
				std::get<pool<decltype(elem)>>(pools_for_components).initialize_space(n);
			};

			for_each_type<components...>(r);
		}

		aggregate_id allocate_aggregate(std::string debug_name = std::string()) {
			auto new_id = pool_for_aggregates.allocate().get_id();
			new_id.set_debug_name(debug_name);

			return new_id;
		}

		aggregate_id clone_aggregate(aggregate_id cloned_aggregate_id) {
			auto cloned_aggregate = pool_for_aggregates.get_handle(cloned_aggregate_id);

			aggregate_handle new_aggregate = pool_for_aggregates.allocate();

			for_each_type<components...>([&cloned_aggregate, &new_aggregate](auto c) {
				auto* maybe_component = cloned_aggregate.find<decltype(c)>();

				if (maybe_component)
					new_aggregate += *maybe_component;
			});

			new_aggregate.set_debug_name(cloned_aggregate.get_debug_name());

			return new_aggregate;
		}

		void free_aggregate(aggregate_id aggregate) {
			auto handle = pool_for_aggregates.get_handle(aggregate);

			for_each_type<components...>([&handle](auto c) {
				handle.remove<decltype(c)>();
			});

			pool_for_aggregates.free(aggregate);
		}
	};

}
