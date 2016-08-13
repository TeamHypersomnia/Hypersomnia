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

	template<class T, bool _or = false>
	void verify_type() {
		static_assert(is_memcpy_safe<T>::value || _or, "Attempt to serialize a non-trivially copyable type");
	}

	template<class A, class T, class...>
	void read_bytes(A& ar, T* location, size_t count) {
		verify_type<T>();
		ar.read(reinterpret_cast<char*>(location), count * sizeof(T));
	}

	template<class A, class T, class...>
	void write_bytes(A& ar, const T* location, size_t count) {
		verify_type<T>();
		ar.write(reinterpret_cast<const char*>(location), count * sizeof(T));
	}

	template<class T, class...>
	void read_bytes(RakNet::BitStream& ar, T* location, size_t count) {
		verify_type<T>();
		ar.Read(reinterpret_cast<char*>(location), count * sizeof(T));
	}

	template<class T, class...>
	void write_bytes(RakNet::BitStream& ar, const T* location, size_t count) {
		verify_type<T>();
		ar.Write(reinterpret_cast<const char*>(location), count * sizeof(T));
	}

	template<class A, class T, class...>
	void read_object(A& ar, T& storage) {
		read_bytes(ar, &storage, 1);
	}

	template<class A, class T, class...>
	void write_object(A& ar, const T& storage) {
		write_bytes(ar, &storage, 1);
	}

	template<class T, class... Args>
	void read_object(RakNet::BitStream& ar, T& storage, Args... args) {
		verify_type<T, std::is_same<T, RakNet::BitStream>::value>();
		ar.Read(storage, args...);
	}

	template<class T, class... Args>
	void write_object(RakNet::BitStream& ar, const T& storage, Args... args) {
		verify_type<T, std::is_same<T, RakNet::BitStream>::value>();
		ar.Write(storage, args...);
	}

	template<class... Args>
	void write_object(RakNet::BitStream& ar, RakNet::BitStream& storage, Args... args) {
		ar.Write(storage, args...);
	}

	template<class A, class T, class...>
	void read_object(A& ar, std::vector<T>& storage) {
		size_t s;
		read_object(ar, s);

		storage.resize(s);
		read_bytes(ar, storage.data(), storage.size()); 
	}

	template<class A, class T, class...>
	void write_object(A& ar, const std::vector<T>& storage) {
		write_object(ar, storage.size());
		write_bytes(ar, storage.data(), storage.size());
	}

	template<class A, class T, class...>
	void read_vector_short(A& ar, std::vector<T>& storage) {
		unsigned short s;
		read_object(ar, s);

		storage.resize(s);
		read_bytes(ar, storage.data(), storage.size());
	}

	template<class A, class T, class...>
	void write_vector_short(A& ar, const std::vector<T>& storage) {
		ensure(storage.size() <= std::numeric_limits<unsigned short>::max());

		write_object(ar, static_cast<unsigned short>(storage.size()));
		write_bytes(ar, storage.data(), storage.size());
	}

	template<class A, class T, class...>
	void read_with_capacity(A& ar, std::vector<T>& storage) {
		size_t c;
		size_t s;

		read_object(ar, c);
		read_object(ar, s);

		storage.reserve(c);
		storage.resize(s);

		read_bytes(ar, storage.data(), storage.size());

	}

	template<class A, class T, class...>
	void write_with_capacity(A& ar, const std::vector<T>& storage) {
		write_object(ar, storage.capacity());
		write_object(ar, storage.size());

		write_bytes(ar, storage.data(), storage.size());
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