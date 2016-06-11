#pragma once
#include "misc/object_pool.h"
#include "configurable_components.h"
#include "ensure.h"
#include "templates.h"

namespace augs {
	namespace detail {
		template<class T>
		struct make_object_pool_id { typedef typename object_pool<T>::typed_id type; };

		template<class T>
		struct make_object_pool { typedef object_pool<T> type; };
	}

	template <class... components>
	class storage_for_components_and_aggregates {
	public:
		class component_aggregate {
			typename transform_types<std::tuple, typename detail::make_object_pool_id, components...>::type component_ids;
			template<class...> friend class component_allocator;
			friend class component_cloner;

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
		};

	public:
		typedef typename object_pool<component_aggregate>::typed_id aggregate_id;

	private:
		template<class component, class... Args>
		auto allocate_component(Args... args) {
			return std::get<augs::object_pool<component>>(storage.pools_for_components).allocate(args...);
		}

		template <class component>
		auto& writable_id(component_aggregate& from) {
			return std::get<typename augs::object_pool<component>::typed_id>(from.data);
		}

		template<class... configured_components>
		class component_allocator {
		public:
			storage_for_components_and_aggregates& storage;
			configurable_components<configured_components...> from;
			component_aggregate to;

			template <class component>
			void operator()() {
				if (from.is_enabled<component>()) {
					writable_id<component>(to) = storage.allocate_component(from.get<component>());
				}
			}
		};

		class component_cloner {
		public:
			storage_for_components_and_aggregates& storage;
			component_aggregate& from;
			component_aggregate& to;

			template <class component>
			void operator()() {
				auto* maybe_component = from.find<component>();

				if (maybe_component) {
					ensure(to.find<component>() == nullptr);

					writable_id<component>(to) = storage.allocate_component(*maybe_component);
				}
			}
		};

	public:
		template<class... configured_components>
		aggregate_id allocate_configured_components(const configurable_components<configured_components...>& configuration) {
			component_allocator allocator{ *this };
			allocator.from = configuration;

			for_each_in_tuple(from.data, allocator);

			return pool_for_aggregates.allocate(allocator.to);
		}

		aggregate_id clone_aggregate(aggregate_id aggregate);
		void free_aggregate(aggregate_id aggregate);
		
		typename transform_types<std::tuple, detail::make_object_pool, components...>::type pools_for_components;
		object_pool<component_aggregate> pool_for_aggregates;
	};

}
