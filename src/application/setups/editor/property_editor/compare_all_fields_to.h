#pragma once
#include <memory>
#include <type_traits>

#include "augs/templates/traits/container_traits.h"
#include "augs/enums/callback_result.h"
#include "augs/templates/introspection_utils/introspective_equal.h"

namespace augs {
	struct trivial_type_marker;
};

template <class M, class id_type, class... Args>
bool compare_all_fields_to(
	const M& first,
	const id_type& property_id,
	Args&&... args
) {
	bool equal = true;

	auto callback = [&](const auto& resolved) -> callback_result {
		using T = std::decay_t<decltype(resolved)>;
		
		if constexpr(std::is_same_v<T, M>) {
			if (!augs::equal_or_introspective_equal(first, resolved)) {
				equal = false;
				return callback_result::ABORT;
			}

			return callback_result::CONTINUE;
		}
		else if constexpr(std::is_same_v<T, augs::trivial_type_marker>) {
			const auto& second = *reinterpret_cast<const M*>(std::addressof(resolved));

			if (!augs::equal_or_introspective_equal(first, second)) {
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
			return callback_result::CONTINUE;
		}
	};

	property_id.access(
		std::forward<Args>(args)...,
		callback
	);

	return equal;
}
