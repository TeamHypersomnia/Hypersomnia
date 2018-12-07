#pragma once
#include <variant>

template <class T>
bool holds_monostate(const T& v) {
	/* Shortcut */
	return std::holds_alternative<std::monostate>(v);
}
