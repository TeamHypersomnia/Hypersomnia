#pragma once
#include "misc/object_pool_handle.h"
#include "component_aggregate.h"

namespace augs {
	template<bool is_const, class derived>
	class component_setters {
	public:
		template<class component,
			class = typename std::enable_if<!is_const>::type>
			decltype(auto) set(const component& c) const {
			if (derived::has<component>())
				return derived::get<component>() = c;
			else
				return derived::add(c);
		}

		template<class component,
			class = typename std::enable_if<!is_const>::type>
			decltype(auto) operator+=(const component& c) const {
			return derived::add(c);
		}

		template<class... added_components,
			class = typename std::enable_if<!is_const>::type>
			void set(added_components... args) const {
			auto components_tuple = std::make_tuple(args...);

			for_each_type<added_components...>([this, &components_tuple](auto c) {
				derived::set(std::get<decltype(c)>(components_tuple));
			});
		}
	};

	template <bool is_const, class derived>
	class component_allocators {
	public:
		template<class component>
		typename maybe_const_ptr<is_const, component>::type find() const {
			derived& self = *static_cast<derived*>(this);

			auto& aggregate = self.get();

			auto& component_pool = self.owner.get_component_pool<component>();
			auto component_handle = component_pool.get_handle(aggregate.get_id<component>());

			if (component_handle.alive())
				return &component_handle.get();

			return nullptr;
		}

		template<class component>
		bool has() const {
			return find<component>() != nullptr;
		}

		template<class component>
		typename maybe_const_ref<is_const, component>::type get() const {
			return *find<component>();
		}

		template<class component,
			class = typename std::enable_if<!is_const>::type>
			component& add(const component& c) const {
			derived& self = *static_cast<derived*>(this);
			ensure(!has<component>());
			self.get().writable_id<component>() = owner.get_component_pool<component>().allocate(c);
		}

		template<class component,
			class = typename std::enable_if<!is_const>::type>
			void remove() const {
			ensure(has<component>());
			derived& self = *static_cast<derived*>(this);
			self.owner.get_component_pool<component>().free(self.get().get_id<component>());
		}
	};
}