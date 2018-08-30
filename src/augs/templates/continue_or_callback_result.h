#pragma once
#include <type_traits>
#include <utility>

#include "augs/enums/callback_result.h"

template <class F, class... Args>
decltype(auto) continue_or_callback_result(F&& callback, Args&&... args) {
	using R = decltype(callback(std::forward<Args>(args)...));
	if constexpr(std::is_same_v<void, R>) {
		callback(std::forward<Args>(args)...);
		return callback_result::CONTINUE;
	}
	else {
		static_assert(std::is_same_v<R, callback_result>, "Bad return type for a for_each callback.");
		return callback(std::forward<Args>(args)...);
	}
}

template <class F, class... Args>
decltype(auto) continue_or_recursive_callback_result(F&& callback, Args&&... args) {
	using R = decltype(callback(std::forward<Args>(args)...));
	if constexpr(std::is_same_v<void, R>) {
		callback(std::forward<Args>(args)...);
		return recursive_callback_result::CONTINUE_AND_RECURSE;
	}
	else {
		static_assert(std::is_same_v<R, recursive_callback_result>, "Bad return type for a for_each callback.");
		return callback(std::forward<Args>(args)...);
	}
}
