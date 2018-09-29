#pragma once
#include "augs/templates/introspect_declaration.h"
#include "augs/templates/remove_cref.h"

template <
	class destination_type,
	class source_type,
	class To, 
	class From, 
	class T
>
void detail_rewriter(
	To& rewritten_to,
	const From& rewritten_from,
	T&& transformator
) {
	if constexpr(
		std::is_same_v<To, destination_type>
		&& std::is_same_v<From, source_type>
	) {
		transformator(rewritten_to, rewritten_from);
	}
	else if constexpr(std::is_assignable_v<To&, const From&>) {
		rewritten_to = rewritten_from;
	}
	else if constexpr(is_std_array_v<From>) {
		for (std::size_t i = 0; i < rewritten_from.size(); ++i) {
			detail_rewriter<destination_type, source_type>(rewritten_to[i], rewritten_from[i], std::forward<T>(transformator));
		}
	}
	else {
		augs::introspect(
			[&](auto, auto& to, const auto& from) {
				detail_rewriter<destination_type, source_type>(to, from, std::forward<T>(transformator));
			}, 
			rewritten_to, 
			rewritten_from
		);
	}
}

template <
	class destination_type,
	template <class> class T,
	class source_type,
	class F
>
T<destination_type> rewrite_members_and_transform_templated_type_into(
	const T<source_type>& source,
	F&& transformator
) {
	static_assert(!std::is_same_v<source_type, destination_type>);

	T<destination_type> destination;

	augs::introspect(
		[&](auto, auto& rewritten_to, const auto& rewritten_from) {
			detail_rewriter<destination_type, source_type>(rewritten_to, rewritten_from, std::forward<F>(transformator));
		},
		destination, 
		source
	);

	return destination;
}