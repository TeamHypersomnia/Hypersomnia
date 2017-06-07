#pragma once
#include "augs/templates/introspect.h"

namespace templates_detail {
	template <class Dest, class Source>
	struct rewriter {
		template <class F>
		static auto f(F&& t) {
			return [&](auto, auto& rewritten_to, const auto& rewritten_from) {
				using To = std::decay_t<decltype(rewritten_to)>;
				using From = std::decay_t<decltype(rewritten_from)>;

				if constexpr(
					std::is_same_v<To, Dest>
					&& std::is_same_v<From, Source>
				) {
					t(rewritten_to, rewritten_from);
				}
				else if constexpr(std::is_assignable_v<To&, const From&>) {
					rewritten_to = rewritten_from;
				}
				else {
					augs::introspect(f(std::forward<F>(t)), rewritten_to, rewritten_from);
				}
			};
		}
	};
}

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
		templates_detail::rewriter<destination_type, source_type>::f(
			std::forward<F>(transformator)
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