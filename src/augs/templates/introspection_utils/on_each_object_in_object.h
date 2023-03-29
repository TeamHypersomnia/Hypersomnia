#pragma once
#include "augs/templates/introspection_utils/on_dynamic_content.h"
#include "augs/templates/identity_templates.h"

/*
	This one is simpler than find_object_in_object
	because we don't need to pass textual description of the entire location.
*/

namespace augs {
	template <template <class> class ShouldRecurse = always_true, class T, class F>
	void on_each_object_in_object(T& object, F&& callback) {
		augs::introspect(
			[&callback](const auto&, auto& member) {
				using Field = remove_cref<decltype(member)>;

				callback(member);

				if constexpr(ShouldRecurse<Field>::value) {
					if constexpr(augs::has_dynamic_content_v<Field>) {
						augs::on_dynamic_content(
							[&](auto& dyn, auto...) {
								callback(dyn);

								using D = remove_cref<decltype(dyn)>;

								if constexpr(!is_introspective_leaf_v<D>) {
									on_each_object_in_object<ShouldRecurse>(dyn, std::forward<F>(callback));
								}
							},
							member
						);
					}
					else {
						if constexpr(!is_introspective_leaf_v<Field>) {
							on_each_object_in_object<ShouldRecurse>(member, std::forward<F>(callback));
						}
					}
				}
			},
			object
		);
	}
}
