#pragma once
#include "misc/object_pool.h"
#include "configurable_components.h"
#include "ensure.h"
#include "templates.h"

namespace templates_detail {
	template<class T>
	struct make_object_pool_id { typedef typename augs::object_pool<T>::typed_id type; };

	template<class T>
	struct make_object_pool { typedef augs::object_pool<T> type; };
}

namespace augs {
	template <class... components>
	class storage_for_components_and_aggregates {
		class component_aggregate {
			typename transform_types<std::tuple, typename templates_detail::make_object_pool_id, components...>::type component_ids;
			friend storage_for_components_and_aggregates;

		public:
			template<class component>
			component* find() {
				auto id = std::get<typename object_pool<component>::typed_id>(component_ids);

				if (id.alive())
					return id.ptr();

				return nullptr;
			}

			template<class component>
			component& get() {
				return *find<component>();
			}

			template<class component>
			const component* find() const {
				auto id = std::get<typename object_pool<component>::typed_id>(component_ids);

				if (id.alive())
					return id.ptr();

				return nullptr;
			}

			template<class component>
			const component& get() const {
				return *find<component>();
			}

			template<class component>
			bool has() const {
				return find<component>() != nullptr;
			}

			const configurable_components<components...>& get_definition() const;
		};

		template<class component, class... Args>
		auto allocate_component(Args... args) {
			return std::get<augs::object_pool<component>>(pools_for_components).allocate(args...);
		}

		template <class component>
		auto& writable_id(component_aggregate& from) {
			return std::get<typename augs::object_pool<component>::typed_id>(from.component_ids);
		}
		
		typename transform_types<std::tuple, templates_detail::make_object_pool, components...>::type pools_for_components;
		object_pool<component_aggregate> pool_for_aggregates;

	public:
		typedef typename object_pool<component_aggregate>::typed_id aggregate_id;

		size_t aggregates_count() const;
		void reserve_storage_for_aggregates(size_t n);

		template<class... configured_components>
		aggregate_id allocate_configured_components(const configurable_components<configured_components...>& configuration) {
			component_aggregate aggregate;

			auto r = [this, &configuration, &aggregate](auto c) {
				if (configuration.is_enabled<decltype(c)>()) {
					writable_id<decltype(c)>(aggregate) = allocate_component<decltype(c)>(configuration.get<decltype(c)>());
				}
			};

			for_each_type<configured_components...>(r);

			return pool_for_aggregates.allocate(aggregate);
		}

		aggregate_id clone_aggregate(aggregate_id aggregate);
		void free_aggregate(aggregate_id aggregate);
	};

}
