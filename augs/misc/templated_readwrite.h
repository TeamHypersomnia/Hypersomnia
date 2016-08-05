#pragma once
#include "augs/templates.h"

namespace augs {
	template<class>
	class basic_pool;

	template<class>
	class pool;

	template<class, typename...>
	class pool_with_meta;

	template<class A, class T, class...>
	void read_object(A& ar, T& storage) {
		static_assert(is_memcpy_safe<T>::value, "Attempt to read a non-trivially copyable type");
		ar.read(reinterpret_cast<char*>(&storage), sizeof(T));
	}

	template<class A, class T, class...>
	void write_object(A& ar, const T& storage) {
		static_assert(is_memcpy_safe<T>::value, "Attempt to write a non-trivially copyable type");
		ar.write(reinterpret_cast<const char*>(&storage), sizeof(T));
	}

	template<class A, class T, class...>
	void read_object(A& ar, std::vector<T>& storage) {
		size_t c;
		size_t s;

		read_object(ar, c);
		read_object(ar, s);

		storage.reserve(c);
		storage.resize(s);

		static_assert(is_memcpy_safe<T>::value, "Attempt to read a non-trivially copyable type");
		ar.read(reinterpret_cast<char*>(storage.data()), storage.size() * sizeof(T));
	}

	template<class A, class T, class...>
	void write_object(A& ar, const std::vector<T>& storage) {
		write_object(ar, storage.capacity());
		write_object(ar, storage.size());

		static_assert(is_memcpy_safe<T>::value, "Attempt to write a non-trivially copyable type");
		ar.write(reinterpret_cast<const char*>(storage.data()), storage.size() * sizeof(T));
	}

	template<class A, class T, class...>
	void read_object(A& ar, basic_pool<T>& storage) {
		storage.read_object(ar);
	}

	template<class A, class T, class...>
	void write_object(A& ar, const basic_pool<T>& storage) {
		storage.write_object(ar);
	}

	template<class A, class T, class...>
	void read_object(A& ar, pool<T>& storage) {
		storage.read_object(ar);
	}

	template<class A, class T, class...>
	void write_object(A& ar, const pool<T>& storage) {
		storage.write_object(ar);
	}

	template<class A, class T, class... Args>
	void read_object(A& ar, pool_with_meta<T, Args...>& storage) {
		storage.read_object(ar);
	}

	template<class A, class T, class... Args>
	void write_object(A& ar, const pool_with_meta<T, Args...>& storage) {
		storage.write_object(ar);
	}

	template<class A, class T, class... Args>
	void read_object(A& ar, std::tuple<T, Args...>& storage) {
		for_each_in_tuple(storage, [&ar](auto& element) {
			read_object(ar, element);
		});
	}

	template<class A, class T, class... Args>
	void write_object(A& ar, const std::tuple<T, Args...>& storage) {
		for_each_in_tuple(storage, [&ar](const auto& element) {
			write_object(ar, element);
		});
	}
}