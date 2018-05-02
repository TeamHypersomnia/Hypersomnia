#pragma once
#include "augs/templates/identity_templates.h"

#include "augs/templates/introspection_utils/introspect_with_containers.h"
#include "augs/templates/introspection_utils/field_name_tracker.h"
#include "augs/templates/introspection_utils/types_in.h"

template <template <class> class IgnorePredicate = always_false, class Se, class O, class F>
void find_object_in_object(
	const Se& searched_object,
	const O& in_object,
	F location_callback
) {
	using S = std::remove_const_t<std::remove_reference_t<Se>>;

	static_assert(can_type_contain_another_v<O, Se>, "This search will never find anything.");

	thread_local augs::field_name_tracker fields;
	fields.clear();

	auto callback = augs::recursive(
		[&searched_object, &location_callback](auto&& self, const auto& label, auto& field) {
			using T = remove_cref<decltype(field)>;

			if constexpr(can_type_contain_another_v<T, Se>) {
				if constexpr(IgnorePredicate<T>::value) {
					/* Apparently, this has a special logic of finding */
				}
				else if constexpr(std::is_same_v<T, S>) {
					if (searched_object == field) {
						location_callback(fields.get_full_name(label));
					}
				}
				else if constexpr(is_introspective_leaf_v<T>) {
					return;
				}
				else if constexpr(augs::has_dynamic_content_v<T>) {
					augs::on_dynamic_content(
						[&](auto& dyn, auto... args) {
							auto scope = fields.track(typesafe_sprintf("%x", args...));
							self(self, "", dyn);
						},
						field
					);
				}
				else {
					auto scope = fields.track(label);
					augs::introspect(augs::recursive(self), field);
				}
			}
		}
	);

	augs::introspect(callback, in_object);
}
