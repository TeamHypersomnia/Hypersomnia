#pragma once
#include "augs/templates/introspect_declaration.h"

template <
	class destination_type,
	template <class> class T,
	class source_type,
	class F
>
T<destination_type> rewrite_members_and_transform_templated_type_into(
	const T<source_type>& source,
	F&& transformator,
	std::enable_if_t<!std::is_same_v<destination_type, source_type>>* = nullptr
) {
	T<destination_type> destination;

	augs::introspect(
		augs::recursive(
			[&](auto&& self, auto, auto& rewritten_to, const auto& rewritten_from) {
				using To = std::decay_t<decltype(rewritten_to)>;
				using From = std::decay_t<decltype(rewritten_from)>;

				if constexpr(
					std::is_same_v<To, destination_type>
					&& std::is_same_v<From, source_type>
				) {
					transformator(rewritten_to, rewritten_from);
				}
				else if constexpr(std::is_assignable_v<To&, const From&>) {
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
template <
	class destination_type,
	template <class> class T,
	class F
>
T<destination_type> rewrite_members_and_transform_templated_type_into(
	const T<destination_type>& source,
	F transformator
) {
	return source;
}