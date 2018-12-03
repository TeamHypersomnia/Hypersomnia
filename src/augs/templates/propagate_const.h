#pragma once
#include <utility>
#include <type_traits>

/* I'm afraid MSVC might not support propagate_const for a while so here's a minimal implementation. */

namespace augs {
	/* Unsafe (about constructors), incomplete wrapper for compatibility only */
	template <class T>
	class propagate_const;

	template <class T>
	class propagate_const {
		using element_type = std::remove_reference_t<decltype(*std::declval<T&>())>;
		T p;

	public:
		template <class... Args>
		propagate_const(Args&&... args) : p(std::forward<Args>(args)...) {};

		element_type* operator->() {
			return p.operator->();
		}

		const element_type* operator->() const {
			return p.operator->();
		}

		element_type* get() {
			return p.get();
		}

		const element_type* get() const {
			return p.get();
		}

		operator element_type*() {
			return p.get();
		}

		operator const element_type*() const {
			return p.get();
		}
	};

	template <class T>
	class propagate_const<T*> {
		T* p;

	public:
		propagate_const(T* const t = nullptr) : p(t) {};

		T* operator->() {
			return p;
		}

		const T* operator->() const {
			return p;
		}

		T* get() {
			return p;
		}

		const T* get() const {
			return p;
		}

		operator T*() {
			return p;
		}

		operator const T*() const {
			return p;
		}
	};
}
