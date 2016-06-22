#pragma once
#include "templates.h"
#include "misc/object_pool.h"
#include "misc/object_pool_id.h"

#include "aggregate_handle.h"

namespace augs {
	template<class T>
	struct make_object_pool { typedef object_pool<T> type; };

	template <class handle_owner_type, class... components>
	class storage_for_components_and_aggregates {
		typedef component_aggregate<components...> aggregate_type;
		typedef object_pool_id<aggregate_type> aggregate_id;

		//template <bool is_const, class owner_type, class... components>
		//friend class basic_aggregate_handle;

		typedef basic_aggregate_handle<false, handle_owner_type, std::tuple<components...>> aggregate_handle;
		typedef basic_aggregate_handle<true, handle_owner_type, std::tuple<components...>> const_aggregate_handle;


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

		template<class... configured_components>
		aggregate_id allocate_configured_components(const configurable_components<configured_components...>& configuration, std::string debug_name = std::string()) {
			aggregate_type aggregate;

			for_each_type<configured_components...>([this, &configuration, &aggregate](auto c) {
				if (configuration.is_set<decltype(c)>()) {
					aggregate.writable_id<decltype(c)>() = allocate_component<decltype(c)>(configuration.get<decltype(c)>());
				}
			});

			auto new_id = pool_for_aggregates.allocate(aggregate);
			new_id.set_debug_name(debug_name);

			return new_id;
		}

		aggregate_id clone_aggregate(const_aggregate_handle aggregate) {
			auto new_aggregate_id = pool_for_aggregates.allocate();
			auto& new_aggregate = pool_for_aggregates.get(new_aggregate_id);

			auto& from_handle = get_handle(aggregate_id);
			auto& to = new_aggregate;

			auto& aggregate = pool_for_aggregates.get(aggregate_id);
			to.removed_from_processing_subjects = aggregate.removed_from_processing_subjects;

			for_each_type<components...>([this, &from_handle, &to](auto c) {
				auto* maybe_component = from_handle.find<decltype(c)>();

				if (maybe_component)
					to.writable_id<decltype(c)>() = allocate_component<decltype(c)>(*maybe_component);
			});

			new_aggregate_id.set_debug_name(aggregate_id.get_debug_name());

			return new_aggregate_id;
		}

		void free_aggregate(aggregate_id aggregate) {
			pool_for_aggregates.free(aggregate);
		}
	};

}
