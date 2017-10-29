#pragma once
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/component_traits.h"

namespace augs {
	template <class derived, class... components>
	class operations_on_all_components_mixin {
	public:
		template <class size_type>
		void reserve_all_components(const size_type n) {
			auto& self = *static_cast<derived*>(this);

			auto reserver = [&](auto c) {
				using component = decltype(c);

				if constexpr(!is_component_fundamental_v<component>) {
					auto& component_pool = self.template get_component_pool<component>();
					component_pool.reserve(n);
				}
			};

			(reserver(components()), ...);
		}

		template <class... excluded_components, class handle_type>
		void clone_all_components_except(
			const handle_type into,
			const handle_type from 
		) {
			auto cloner = [&](auto c) {
				using component = decltype(c);
				using allocator_base = typename handle_type::allocator;

				if constexpr(!is_one_of_v<component, excluded_components...>) {
					if constexpr(is_component_fundamental_v<component>) {
						into.allocator_base::template get<component>() = from.allocator_base::template get<component>();
					}
					else {
						if (from.allocator_base::template has<component>()) {
							if (into.allocator_base::template has<component>()) {
								into.allocator_base::template get<component>() = from.allocator_base::template get<component>();
							}
							else {
								into.allocator_base::template add<component>(from.allocator_base::template get<component>());
							}
						}
					}
				}
			};
			
			(cloner(components()), ...);
		}

		template <class handle_type>
		void free_all_components(const handle_type handle) {
			auto& self = *static_cast<derived*>(this);

			auto freer = [&](auto c) {
				using component = decltype(c);
				using allocator_base = typename handle_type::allocator;

				if constexpr(!is_component_fundamental_v<component>) {
					if (handle.allocator_base::template has<component>()) {
						handle.allocator_base::template remove<component>();
					}
				}
			};

			(freer(components()), ...);
		}
	};
}
