#pragma once
#include <type_traits>

namespace augs {
	template <class To, class From>
	typename std::enable_if<
		(sizeof(To) == sizeof(From)) &&
		std::is_trivially_copyable<From>::value &&
		std::is_trivial<To>::value,
		// this implementation requires that To is trivially default constructible
		To>::type
	// constexpr support needs compiler magic
	bit_cast(const From &src) noexcept
	{
		To dst;
		std::memcpy(&dst, &src, sizeof(To));
		return dst;
	}
}
