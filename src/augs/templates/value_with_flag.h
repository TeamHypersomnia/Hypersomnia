#pragma once
#include <optional>
#include "augs/templates/traits/is_comparable.h"

namespace augs {
	struct introspection_access;

	template <class T>
	class value_with_flag {
		friend augs::introspection_access;

		// GEN INTROSPECTOR class augs::value_with_flag class T
		T object;
		bool is_used = false;
		// END GEN INTROSPECTOR
	public:
		using value_type = T;

		value_with_flag() = default;
		value_with_flag(std::nullopt_t) {}
		value_with_flag(const value_with_flag&) = default;
		value_with_flag(value_with_flag&& b) = default; 

		value_with_flag(const T& b) : object(b) { is_used = true; }
		value_with_flag(T&& b) : object(std::move(b)) { is_used = true; }

		value_with_flag& operator=(const value_with_flag& b) = default;

		value_with_flag& operator=(std::nullopt_t) {
			is_used = false;

			return *this;
		}

		value_with_flag& operator=(T&& b) {
			object = std::move(b);
			is_used = true;

			return *this;
		}

		value_with_flag& operator=(const T& b) {
			object = b;
			is_used = true;

			return *this;
		}

		template <class... Args>
		void emplace(Args&&... args) {
			object.~T();
			new (std::addressof(object)) T(std::forward<Args>(args)...);
			is_used = true;
		}

		T* operator->() {
			return std::addressof(object);
		}

		const T* operator->() const {
			return std::addressof(object);
		}

		T& operator*() {
			return object;
		}

		const T& operator*() const {
			return object;
		}

		auto& value() {
			return object;
		}

		const auto& value() const {
			return object;
		}

		void reset() {
			is_used = false;
		}

		void restore() {
			is_used = true;
		}

		bool has_value() const {
			return is_used;
		}

		explicit operator bool() const {
			return is_used;
		}

		bool operator==(std::nullopt_t) const {
			return !is_used;
		}

		bool operator!=(std::nullopt_t) const {
			return is_used;
		}
	};
}

template <class T, class = std::enable_if_t<is_comparable_v<T, T>>>
bool operator==(const augs::value_with_flag<T>& op, const T& b) {
	return op.has_value() && op.value() == b;
}

template <class T, class = std::enable_if_t<is_comparable_v<T, T>>>
bool operator==(const augs::value_with_flag<T>& op, const augs::value_with_flag<T>& b) {
	return op.value() == b.value() && op.has_value() == b.has_value();
}

template <class T, class = std::enable_if_t<is_neq_comparable_v<T, T>>>
bool operator!=(const augs::value_with_flag<T>& op, const T& b) {
	return !(op == b);
}

template <class T, class = std::enable_if_t<is_neq_comparable_v<T, T>>>
bool operator!=(const augs::value_with_flag<T>& op, const augs::value_with_flag<T>& b) {
	return !(op == b);
}
