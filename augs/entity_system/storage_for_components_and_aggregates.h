#pragma once
#include "templates.h"
#include "misc/object_pool.h"
#include "misc/object_pool_id.h"

#include "aggregate_mixins.h"

namespace augs {
	template<class T>
	struct make_object_pool { typedef object_pool<T> type; };

	template <class handle_owner_type, class... components>
	class storage_for_components_and_aggregates {
		typedef component_aggregate<components...> aggregate_type;
		typedef object_pool_id<aggregate_type> aggregate_id;

	public:
		typedef object_pool<aggregate_type> aggregate_pool_type;
	private:

		aggregate_pool_type pool_for_aggregates;
		typename transform_types<std::tuple, make_object_pool, components...>::type pools_for_components;

	public:
		const auto& get_pool() const {
			return pool_for_aggregates;
		}

		auto& get_pool() {
			return pool_for_aggregates;
		}

		template<class component>
		const auto& get_component_pool() const {
			return std::get<augs::object_pool<component>>(pools_for_components);
		}

		template<class component>
		auto& get_component_pool() {
			return std::get<augs::object_pool<component>>(pools_for_components);
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
				std::get<object_pool<decltype(elem)>>(pools_for_components).initialize_space(n);
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
