#pragma once
#include "augs/templates/introspect_declaration.h"
#include "augs/templates/remove_cref.h"

template <
	class destination_type,
	template <class> class T,
	class source_type,
	class F
>
T<destination_type> rewrite_members_and_transform_templated_type_into(
	const T<source_type>& source,
	F transformator
) {
	static_assert(!std::is_same_v<source_type, destination_type>);
	T<destination_type> destination;

	augs::introspect(
		augs::recursive(
			[&](auto self, auto, auto& rewritten_to, const auto& rewritten_from) {
				using To = remove_cref<decltype(rewritten_to)>;
				using From = remove_cref<decltype(rewritten_from)>;

				if constexpr(
					std::is_same_v<To, destination_type>
					&& std::is_same_v<From, source_type>
				) {
					(void)self;
					transformator(rewritten_to, rewritten_from);
				}
				else if constexpr(std::is_assignable_v<To&, const From&>) {
					(void)self;
					rewritten_to = rewritten_from;
				}
				else {
					augs::introspect(augs::recursive(self), rewritten_to, rewritten_from);
				}
			}
		), 
		destination, 
		source
	);

	return destination;
}