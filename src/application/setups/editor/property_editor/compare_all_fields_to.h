#pragma once
#include <memory>
#include <type_traits>

#include "augs/templates/traits/container_traits.h"
#include "augs/enums/callback_result.h"
#include "augs/templates/introspection_utils/introspective_equal.h"

namespace augs {
	struct trivial_type_marker;
};

inline auto& detail_bytes_for_first() {
	thread_local std::vector<std::byte> bytes;
	bytes.clear();
	return bytes;
}

inline auto& detail_bytes_for_candidate() {
	thread_local std::vector<std::byte> bytes;
	bytes.clear();
	return bytes;
}

template <class M>
inline auto& detail_bytes_of_first(M& first) {
	auto& b = detail_bytes_for_first();
	auto s = augs::ref_memory_stream(b);
	augs::write_bytes(s, first);
	return b;
}

template <class M>
inline auto& detail_bytes_of_candidate(M& candidate) {
	auto& b = detail_bytes_for_candidate();
	auto s = augs::ref_memory_stream(b);
	augs::write_bytes(s, candidate);
	return b;
}

template <class T>
bool detail_compare(
	const std::vector<std::byte>& first_bytes,
	const T& resolved
) {
	if constexpr(std::is_same_v<T, std::nullptr_t>) {
		/* Not found! */
		return false;
	}
	else if constexpr(std::is_same_v<T, augs::trivial_type_marker>) {
		const auto* const candidate_bytes = reinterpret_cast<const std::byte*>(std::addressof(resolved));
		return !std::memcmp(first_bytes.data(), candidate_bytes, first_bytes.size());
	}
	else {
		return first_bytes == detail_bytes_of_candidate(resolved);
	}

	return true;
}

template <class id_type, class... Args>
bool compare_all_serialized_fields_to(
	const std::vector<std::byte>& first_bytes,
   	const id_type& property_id,
   	Args&&... args
) {
	bool equal = true;

	property_id.access(
		std::forward<Args>(args)...,
		[&first_bytes, &equal](const auto& resolved) -> callback_result {
			if (detail_compare(first_bytes, resolved)) {
				equal = false;
				return callback_result::ABORT;
			}

			return callback_result::CONTINUE;
		}
	);

	return equal;
}

template <class M, class id_type, class... Args>
bool compare_all_fields_to(
	const M& first,
	const id_type& property_id,
	Args&&... args
) {
	return compare_all_serialized_fields_to(
		detail_bytes_of_first(first), 
		property_id, 
		std::forward<Args>(args)...
	);
}
