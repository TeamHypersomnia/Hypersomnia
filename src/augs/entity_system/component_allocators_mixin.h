#pragma once
#include "augs/ensure.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/component_traits.h"

namespace augs {
	template <bool is_const, class derived_entity_handle>
	class component_allocators_mixin {
	public:
		template<class component>
		maybe_const_ptr_t<is_const, component> find() const {
			const auto& self = *static_cast<const derived_entity_handle*>(this);
			auto& aggregate = self.get();

			if constexpr(is_component_fundamental_v<component>) {
				return &std::get<component>(aggregate.fundamentals);
			}
			else {
				const auto component_id = aggregate.template get_id<component>();
				auto& component_pool = self.owner.get_component_pool<component>();

				if (component_id.is_set() && component_pool.alive(component_id)) {
					return &component_pool.get(component_id);
				}

				return nullptr;
			}
		}

		template<class component>
		bool has() const {
			if constexpr(is_component_fundamental_v<component>) {
				return true;
			}
			else {
				return find<component>() != nullptr;
			}
		}

		template<class component>
		maybe_const_ref_t<is_const, component> get() const {
			if constexpr(!is_component_fundamental_v<component>) {
				ensure(has<component>());
			}

			return *find<component>();
		}

		template <
			class component, 
			class = std::enable_if<!is_const>
		>
		void add(const component& c) const {
			if constexpr(is_component_fundamental_v<component>) {
				get<component>() = c;
			}
			else {
				ensure(!has<component>());

				auto& self = *static_cast<const derived_entity_handle*>(this);
				self.get().set_id(self.get_cosmos().template get_component_pool<component>().allocate(c));
			}
		}

		template <
			class component, 
			class = std::enable_if<!is_const && !is_component_fundamental_v<component>>
		>
		void remove() const {
			ensure(has<component>());

			auto& self = *static_cast<const derived_entity_handle*>(this);
			auto& aggregate = self.get();
			const auto id_of_deleted = aggregate.get_id<component>();

			self.get_cosmos().template get_component_pool<component>().free(id_of_deleted);
			aggregate.set_id(decltype(id_of_deleted)());
		}
	};
}
