#pragma once
#include <utility>
#include <type_traits>

namespace augs {
	template <class F>
	auto recursive(F callback) {
		return [callback](auto&&... args) { 
			callback(
				callback, 
				std::forward<decltype(args)>(args)...
			);
		};
	}
}