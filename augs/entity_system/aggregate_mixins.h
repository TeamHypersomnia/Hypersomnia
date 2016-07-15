#pragma once
#include "misc/pool_handle.h"
#include "component_aggregate.h"

namespace augs {
	template<bool is_const, class derived>
	class component_setters {
	public:
		template<class component,
			class = std::enable_if_t<!is_const>>
			decltype(auto) set(const component& c) const {
			auto& self = *static_cast<const derived*>(this);
			if (self.has<component>())
				return self.get<component>() = c;
			else
				return self.add(c);
		}

		template<class component,
			class = std::enable_if_t<!is_const>>
			decltype(auto) operator+=(const component& c) const {
			auto& self = *static_cast<const derived*>(this);
			return self.add(c);
		}

		template<class... added_components,
			class = std::enable_if_t<!is_const>>
			void set(added_components... args) const {
			auto components_tuple = std::make_tuple(args...);

			for_each_type<added_components...>([this, &components_tuple](auto c) {
				set(std::get<decltype(c)>(components_tuple));
			});
		}
	};

	template <bool is_const, class derived>
	class component_allocators {
	public:
		template<class component>
		typename maybe_const_ptr<is_const, component>::type find() const {
			auto& self = *static_cast<const derived*>(this);

			auto& aggregate = self.get();

			auto& component_pool = self.owner.get_pool(pool_id<component>());
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
		maybe_const_ref_t<is_const, component> get() const {
			return *find<component>();
		}

		template<class component,
			class = std::enable_if_t<!is_const>>
			component& add(const component& c) const {
			auto& self = *static_cast<const derived*>(this);
			ensure(!has<component>());
			self.get().writable_id<component>() = self.owner.get_pool(pool_id<component>()).allocate(c);
		}

		template<class component,
			class = std::enable_if_t<!is_const>>
			void remove() const {
			ensure(has<component>());
			auto& self = *static_cast<const derived*>(this);
			self.owner.get_pool(pool_id<component>()).free(self.get().get_id<component>());
		}
	};
}