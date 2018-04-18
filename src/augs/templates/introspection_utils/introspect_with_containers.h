#pragma once
#include "augs/templates/introspect.h"

#include "augs/templates/traits/is_variant.h"
#include "augs/templates/traits/is_optional.h"
#include "augs/templates/traits/container_traits.h"

namespace augs {
	template <class F, class O>
	void introspect_with_containers(F callback, O& object) {
		augs::introspect(
			[&callback](const auto& label, auto& field) {
				using T = std::decay_t<decltype(field)>;

				if constexpr(is_introspective_leaf_v<T>) {
					callback(label, field);
				}
				else if constexpr(is_optional_v<T>) {
					if (field) {
						callback(label, *field);
					}
				}
				else if constexpr(is_variant_v<T>) {
					std::visit(
						[&](auto& resolved){ callback(label, resolved); }, 
						field
					);
				}
				else if constexpr(is_container_v<T>) {
					int i = 0;

					for (auto&& elem : field) {
						callback(typesafe_sprintf("%x.%x", label, i++), field);
					}
				}
				else {
					callback(label, field);
				}
			}, 
			object
		);
	}
}
