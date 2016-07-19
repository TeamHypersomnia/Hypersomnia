#pragma once
#include "augs/misc/pool_handle.h"
#include "component_aggregate.h"
#include "augs/ensure.h"

namespace augs {
	template<bool is_const, class derived>
	class component_setters {
	public:
		template<class component, bool _is_const = is_const,
			class = std::enable_if_t<!_is_const>>
			decltype(auto) set(const component& c) const {
			auto& self = *static_cast<const derived*>(this);
			if (self.template has<component>())
				return self.template get<component>() = c;
			else
				return self.add(c);
		}

		template<class component, bool _is_const = is_const,
			class = std::enable_if_t<!_is_const>>
			decltype(auto) operator+=(const component& c) const {
			auto& self = *static_cast<const derived*>(this);
			return self.add(c);
		}

		template<class... added_components, bool _is_const = is_const,
			class = std::enable_if_t<!_is_const>>
			void set(added_components... args) const {
			auto components_tuple = std::make_tuple(args...);

			for_each_in_tuple(components_tuple, [this](auto& c) {
				set(c);
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
			auto component_handle = component_pool.get_handle(aggregate.template get_id<component>());

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
			ensure(has<component>());
			return *find<component>();
		}

		template<class component, bool _is_const = is_const,
			class = std::enable_if<!_is_const>>
		  void add(const component& c) const {
			auto& self = *static_cast<const derived*>(this);
			ensure(!has<component>());
			self.get().template writable_id<component>() = self.owner.get_pool(pool_id<component>()).allocate(c);
		}

		template<class component, bool _is_const = is_const,
			class = std::enable_if_t<!_is_const>>
			void remove() const {
			ensure(has<component>());
			auto& self = *static_cast<const derived*>(this);
			self.owner.get_pool(pool_id<component>()).free(self.get().template get_id<component>());
		}
	};
}
