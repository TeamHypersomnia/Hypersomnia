#pragma once
#include <memory>
#include <type_traits>

#include "augs/enums/callback_result.h"

namespace augs {
	struct trivial_type_marker;
};


template <class M>
inline auto make_all_fields_comparator(const M& first, bool& equal) {
	return [&](const auto& resolved) {
		using T = std::decay_t<decltype(resolved)>;
		
		if constexpr(std::is_same_v<T, M>) {
			if(!(first == resolved)) {
				equal = false;
				return callback_result::ABORT;
			}

			return callback_result::CONTINUE;
		}
		else if constexpr(std::is_same_v<T, augs::trivial_type_marker>) {
			if(!(first == *reinterpret_cast<const M*>(std::addressof(resolved)))) {
				equal = false;
				return callback_result::ABORT;
			}

			return callback_result::CONTINUE;
		}
		else if constexpr(std::is_same_v<T, std::nullopt_t>) {
			/* Not found! */
			equal = false;
			return callback_result::ABORT;
		}
		else {
			return callback_result::ABORT;
		}
	};
};

template <class M, class id_type, class... Args>
bool compare_all_fields_to(
	const M& first,
	const id_type& property_id,
	Args&&... args
) {
	bool equal = true;

	property_id.access(
		std::forward<Args>(args)...,
		make_all_fields_comparator(first, equal)
	);

	return equal;
}
