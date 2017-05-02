#pragma once
#include <type_traits>

namespace augs {
	template <bool>
	struct conditional_call;

	template <>
	struct conditional_call<true> {
		template <
			class F,
			class... Args
		>
		void operator()(F&& callback, Args&&... args) {
			callback(std::forward<Args>(args)...);
		}
	};

	template <>
	struct conditional_call<false> {
		template <
			class F,
			class... Args
		>
		void operator()(F&& callback, Args&&... args) {

		}
	};
}