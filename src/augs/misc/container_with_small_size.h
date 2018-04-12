#pragma once
#include <limits>
#include <type_traits>

#include "augs/readwrite/byte_readwrite_declaration.h"
#include "augs/templates/traits/container_traits.h"

namespace augs {
	template <class T, class size_type>
	class container_with_small_size {
		T container;
	public:
		void overflow_check() {
			if (overflowed()) {
				clear();
			}
		}

		bool overflowed() const {
			return container.size() > std::numeric_limits<size_type>::max();
		}

		template <class... Args>
		container_with_small_size(Args&&... args) : container(std::forward<Args>(args)...) {
			overflow_check();
		}

		T& operator*() {
			return container;
		}

		const T& operator*() const {
			return container;
		}

		T* operator->() {
			return &container;
		}

		bool is_full() const {
			return container.size() >= std::numeric_limits<size_type>::max();
		}

		const T* operator->() const {
			return &container;
		}

		template <class... Args>
		decltype(auto) insert(Args&&... args) {
			const auto result = container.insert(std::forward<Args>(args)...);
			overflow_check();
			return result;
		}

		template <class... Args>
		decltype(auto) find(Args&&... args) {
			return container.find(std::forward<Args>(args)...);
		}

		template <class... Args>
		decltype(auto) push_back(Args&&... args) {
			container.push_back(std::forward<Args>(args)...);
			overflow_check();
		}
		
		template <class... Args>
		decltype(auto) erase(Args&&... args) {
			return container.erase(std::forward<Args>(args)...);
		}

		template <class... Args>
		decltype(auto) operator[](Args&&... args) {
			return container.operator[](std::forward<Args>(args)...);
		}

		template <class... Args>
		decltype(auto) operator[](Args&&... args) const {
			return container.operator[](std::forward<Args>(args)...);
		}

		decltype(auto) begin() {
			return container.begin();
		}

		decltype(auto) end() {
			return container.end();
		}

		decltype(auto) begin() const {
			return container.begin();
		}

		decltype(auto) end() const {
			return container.end();
		}

		decltype(auto) size() const {
			return container.size();
		}

		template <bool C = can_access_data_v<T>, class = std::enable_if_t<C>>
		decltype(auto) data() {
			return container.data();
		}

		template <bool C = can_access_data_v<T>, class = std::enable_if_t<C>>
		decltype(auto) data() const {
			return container.data();
		}

		void clear() {
			container.clear();
		}
	};

#if READWRITE_OVERLOAD_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

	template<class A, class T, class size_type>
	void read_object_bytes(
		A& ar,
		container_with_small_size<T, size_type>& storage
	) {
		read_container_bytes(ar, *storage, size_type());
	}

	template<class A, class T, class size_type>
	void write_object_bytes(
		A& ar,
		const container_with_small_size<T, size_type>& storage
	) {
		write_container_bytes(ar, *storage, size_type());
	}
}

template <
	class A, 
	class B, 
	class size_type_A, 
	class size_type_B
>
bool operator==(
	const augs::container_with_small_size<A, size_type_A>& a,
	const augs::container_with_small_size<B, size_type_B>& b
) {
	return *a == *b;
}