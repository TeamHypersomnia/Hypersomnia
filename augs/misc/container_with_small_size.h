#pragma once
#include <limits>
#include "augs/ensure.h"

namespace augs {
	template <class T, class size_type>
	class container_with_small_size {
		T container;
	public:

		T& operator*() {
			return container;
		}

		const T& operator*() const {
			return container;
		}

		T* operator->() {
			return &container;
		}

		void size_check() const {
			ensure(container.size() < std::numeric_limits<size_type>::max());
		}

		const T* operator->() const {
			return &container;
		}

		template <class... Args>
		decltype(auto) insert(Args&&... args) {
			const auto result =  container.insert(std::forward<Args>(args)...);
			size_check();
			return result;
		}

		template <class... Args>
		decltype(auto) push_back(Args&&... args) {
			const auto result = container.push_back(std::forward<Args>(args)...);
			size_check();
			return result;
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
			return container.begin();
		}

		decltype(auto) begin() const {
			return container.begin();
		}

		decltype(auto) end() const {
			return container.begin();
		}
	};

	template<class A, class T, class size_type>
	void read_object(
		A& ar,
		container_with_small_size<T, size_type>& storage
	) {
		read_object(ar, *storage, size_type());
	}

	template<class A, class T, class size_type>
	void write_object(
		A& ar,
		const container_with_small_size<T, size_type>& storage
	) {
		write_object(ar, *storage, size_type());
	}
}