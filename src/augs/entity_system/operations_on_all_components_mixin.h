#pragma once
#include "augs/templates/for_each_type.h"
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/component_traits.h"

namespace augs {
	template <class derived, class... components>
	class operations_on_all_components_mixin {
	public:
		template <class size_type>
		void reserve_all_components(const size_type n) {
			auto& self = *static_cast<derived*>(this);

			auto reserver = [&self, n](auto c) {
				using component = decltype(c);

				if constexpr(!is_component_fundamental_v<component>) {
					auto& component_pool = self.template get_component_pool<component>();
					component_pool.reserve(n);
				}
			};

			for_each_type<components...>(reserver);
		}

		template <class... excluded_components, class handle_type>
		void clone_all_components_except(
			const handle_type into,
			const handle_type from 
		) {
			for_each_type<components...>([&from, &into](auto c) {
				using component = decltype(c);

				if constexpr(!is_one_of_v<component, excluded_components...>) {
					if constexpr(is_component_fundamental_v<component>) {
						into.allocator::template get<component>() = from.allocator::template get<component>();
					}
					else {
						if (from.allocator::template has<component>()) {
							if (into.allocator::template has<component>()) {
								into.allocator::template get<component>() = from.allocator::template get<component>();
							}
							else {
								into.allocator::template add<component>(from.allocator::template get<component>());
							}
						}
					}
				}
			});
		}

		template <class handle_type>
		void free_all_components(const handle_type handle) {
			auto& self = *static_cast<derived*>(this);

			for_each_type<components...>([&](auto c) {
				using component = decltype(c);

				if constexpr(!is_component_fundamental_v<component>) {
					if (handle.allocator::template has<component>()) {
						handle.allocator::template remove<component>();
					}
				}
			});
		}
	};
}
