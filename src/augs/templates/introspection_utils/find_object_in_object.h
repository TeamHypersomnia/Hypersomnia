#pragma once
#include "augs/templates/identity_templates.h"

#include "augs/templates/introspection_utils/introspect_with_containers.h"
#include "augs/templates/introspection_utils/field_name_tracker.h"
#include "augs/templates/introspection_utils/types_in.h"

template <bool allow_conversion>
struct detail_same_or_convertible;

template <>
struct detail_same_or_convertible<false> {
	template <class A, class B>
	static constexpr bool value = can_type_contain_v<A, B>;
};

template <>
struct detail_same_or_convertible<true> {
	template <class A, class B>
	static constexpr bool value = can_type_contain_constructible_from_v<A, B>;
};

template <
	template <class> class IgnorePredicate = always_false, 
	bool allow_conversion = false,
	class Searched, 
	class O, 
	class F
>
void find_object_in_object(
	const Searched& searched_object,
	const O& in_object,
	F location_callback
) {
	using contains = detail_same_or_convertible<allow_conversion>;
	static_assert(contains::template value<O, Searched>, "This search will never find anything.");

	thread_local augs::field_name_tracker fields;
	fields.clear();

	auto callback = augs::recursive(
		[&searched_object, &location_callback](auto&& self, const auto& label, auto& field) {
			using Candidate = remove_cref<decltype(field)>;

			if constexpr(contains::template value<Candidate, Searched>) {
				if constexpr(IgnorePredicate<Candidate>::value) {
					/* Apparently, this has a special logic of finding */
				}
				else if constexpr(std::is_same_v<Candidate, Searched>) {
					if (searched_object == field) {
						location_callback(fields.get_full_name(label));
					}
				}
				else if constexpr(allow_conversion && std::is_constructible_v<Candidate, Searched>) {
					if (Candidate(searched_object) == field) {
						location_callback(fields.get_full_name(label));
					}
				}
				else if constexpr(is_introspective_leaf_v<Candidate>) {
					return;
				}
				else if constexpr(augs::has_dynamic_content_v<Candidate>) {
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
