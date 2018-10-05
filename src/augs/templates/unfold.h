#pragma once
#include <type_traits>

template <class... Types, class List, class F>
void unfold(List&& l, F&& callback) {
	auto f = [&](auto* dummy) {
		using T = std::remove_pointer_t<decltype(dummy)>;
		callback(std::get<T>(l));
	};

	(f((Types*)(nullptr)), ...);
}

template <template <class> class Mod, class... Types, class List, class F>
void unfold(List&& l, F&& callback) {
	auto f = [&](auto* dummy) {
		using T = Mod<std::remove_pointer_t<decltype(dummy)>>;
		callback(std::get<T>(l));
	};

	(f((Types*)(nullptr)), ...);
}
