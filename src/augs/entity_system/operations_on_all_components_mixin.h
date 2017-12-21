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

		template <class... excluded_components, class aggregate_type>
		void clone_all_components_except(
			aggregate_type& into,
			const aggregate_type& from 
		) {
			auto& self = *static_cast<derived*>(this);

			auto cloner = [&](auto c) {
				using component = decltype(c);

				if constexpr(!is_one_of_v<component, excluded_components...>) {
					into.template get<component>(self) = from.template get<component>(self);
				}
			};
			
			(cloner(components()), ...);
		}

		template <class aggregate_type>
		void free_all_components(aggregate_type& agg) {
			auto& self = *static_cast<derived*>(this);

			auto freer = [&](auto c) {
				using component = decltype(c);

				if constexpr(!is_component_fundamental_v<component>) {
					if (agg.template has<component>(self)) {
						agg.template remove<component>(self);
					}
				}
			};

			(freer(components()), ...);
		}
	};
}
