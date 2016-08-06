#pragma once
#include "augs/templates.h"
#include "augs/ensure.h"

namespace RakNet {
	class BitStream;
}

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

	template<class T, class...>
	void read_object(RakNet::BitStream& ar, T& storage) {
		static_assert(std::is_same<T, RakNet::BitStream>::value || is_memcpy_safe<T>::value, "Attempt to read a non-trivially copyable type");
		ar.Read(storage);
	}

	template<class T, class...>
	void write_object(RakNet::BitStream& ar, const T& storage) {
		static_assert(std::is_same<T, RakNet::BitStream>::value || is_memcpy_safe<T>::value, "Attempt to write a non-trivially copyable type");
		ar.Write(storage);
	}

	template<class A, class T, class...>
	void read_objects(A& ar, T* storage, size_t count) {
		static_assert(is_memcpy_safe<T>::value, "Attempt to read a non-trivially copyable type");
		ar.read(reinterpret_cast<char*>(&storage), sizeof(T)*count);
	}

	template<class A, class T, class...>
	void write_objects(A& ar, const T* storage, size_t count) {
		static_assert(is_memcpy_safe<T>::value, "Attempt to write a non-trivially copyable type");
		ar.write(reinterpret_cast<const char*>(&storage), sizeof(T)*count);
	}

	template<class A, class T, class...>
	void read_object(A& ar, std::vector<T>& storage) {
		size_t s;
		read_object(ar, s);

		storage.resize(s);
		read_objects(ar, storage.data(), storage.size()); 
	}

	template<class A, class T, class...>
	void write_object(A& ar, const std::vector<T>& storage) {
		write_object(ar, storage.size());
		write_objects(ar, storage.data(), storage.size());
	}

	template<class A, class T, class...>
	void read_vector_short(A& ar, std::vector<T>& storage) {
		unsigned short s;
		read_object(ar, s);

		storage.resize(s);
		read_objects(ar, storage.data(), storage.size());
	}

	template<class A, class T, class...>
	void write_vector_short(A& ar, const std::vector<T>& storage) {
		ensure(storage.size() <= std::numeric_limits<unsigned short>::max());

		write_object(ar, static_cast<unsigned short>(storage.size()));
		write_objects(ar, storage.data(), storage.size());
	}

	template<class A, class T, class...>
	void read_with_capacity(A& ar, std::vector<T>& storage) {
		size_t c;
		size_t s;

		read_object(ar, c);
		read_object(ar, s);

		storage.reserve(c);
		storage.resize(s);

		read_objects(ar, storage.data(), storage.size());

	}

	template<class A, class T, class...>
	void write_with_capacity(A& ar, const std::vector<T>& storage) {
		write_object(ar, storage.capacity());
		write_object(ar, storage.size());

		write_objects(ar, storage.data(), storage.size());
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