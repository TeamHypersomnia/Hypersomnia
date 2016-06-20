#pragma once
#include "misc/object_pool.h"
#include "configurable_components.h"
#include "ensure.h"
#include "templates.h"
#include <vector>

namespace augs {
	template<class T>
	struct make_object_pool { typedef object_pool<T> type; };

	template<class T>
	struct make_object_pool_id { typedef object_pool_id<T> type; };

	template <class... components>
	class storage_for_components_and_aggregates;

	template <class... components>
	class component_aggregate {
		typename transform_types<std::tuple, make_object_pool_id, components...>::type component_ids;

		template <class component>
		auto& get_id() {
			return std::get<typename augs::object_pool_id<component>>(from.component_ids);
		}

		friend class storage_for_components_and_aggregates<components...>;

	public:
		unsigned long long removed_from_processing_subjects = 0;
	};

	template <class... components>
	class storage_for_components_and_aggregates {
		typedef component_aggregate<components...> aggregate_type;

		template<class component, class... Args>
		auto allocate_component(Args... args) {
			return std::get<augs::object_pool<component>>(pools_for_components).allocate(args...);
		}

		typename transform_types<std::tuple, make_object_pool, components...>::type pools_for_components;
		object_pool<aggregate_type> pool_for_aggregates;

	public:
		typedef typename object_pool_id<aggregate_type> aggregate_id;

		template <bool is_const>
		class basic_aggregate_handle {
			typedef typename std::conditional<is_const, const storage_for_components_and_aggregates&, storage_for_components_and_aggregates&>::type pool_reference;

		public:
			pool_reference owner;
			aggregate_id raw_id;

			basic_aggregate_handle(pool_reference own, aggregate_id raw)
				: owner(own), raw_id(raw) {}

			template<class component>
			typename std::conditional<is_const, const component*, component*>::type find() const {
				auto& aggregate = owner.pool_for_aggregates.get(raw_id);

				auto component_id = aggregate.get_id<component>();
				auto& component_pool = std::get<typename object_pool<component>>(owner.pools_for_components);
				auto component_handle = component_pool.get_handle(component_id);

				if (component_handle.alive())
					return &component_handle.get();

				return nullptr;
			}

			template<class component>
			typename std::conditional<is_const, const component&, component&>::type get() const {
				return *find<component>();
			}

			template<class component>
			bool has() const {
				return find<component>() != nullptr;
			}

			template<class component,
				class = typename std::enable_if<!is_const>::type>
			component& set(const component& c) const {
				get<component>() = c;
				return get<component>();
			}

			template<class... components,
				class = typename std::enable_if<!is_const>::type>
			void set(components... args) const {
				auto components_tuple = std::make_tuple(args...);

				for_each_type<components...>([this, &components_tuple](auto c) {
					set(std::get<decltype(c)>(components_tuple));
				});
			}

			aggregate_id get_id() const;

			const configurable_components<components...>& get_definition() const;
		};

		typedef basic_aggregate_handle<false> aggregate_handle;
		typedef basic_aggregate_handle<true> const_aggregate_handle;

		size_t aggregates_count() const;
		void reserve_storage_for_aggregates(size_t n);

		aggregate_handle get_handle(aggregate_id id) {
			return{ *this, id };
		}

		const_aggregate_handle get_handle(aggregate_id id) const {
			return{ *this, id };
		}

		template<class... configured_components>
		aggregate_id allocate_configured_components(const configurable_components<configured_components...>& configuration, std::string debug_name = std::string()) {
			component_aggregate aggregate;

			for_each_type<configured_components...>([this, &configuration, &aggregate](auto c) {
				if (configuration.is_set<decltype(c)>()) {
					aggregate.get_id<decltype(c)>() = allocate_component<decltype(c)>(configuration.get<decltype(c)>());
				}
			});

			auto new_id = pool_for_aggregates.allocate(aggregate);
			new_id.set_debug_name(debug_name);

			return new_id;
		}

		aggregate_id clone_aggregate(aggregate_id aggregate);
		void free_aggregate(aggregate_id aggregate);
	};

}
