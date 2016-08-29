#pragma once
#include "augs/templates.h"
#include "augs/ensure.h"

namespace augs {
	class stream;
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
	auto read_bytes(A& ar, T* location, size_t count) {
		verify_type<T>();
		return ar.read(reinterpret_cast<char*>(location), count * sizeof(T));
	}

	template<class A, class T, class...>
	void write_bytes(A& ar, const T* location, size_t count) {
		verify_type<T>();
		ar.write(reinterpret_cast<const char*>(location), count * sizeof(T));
	}

	template<class T, class...>
	bool read_bytes(std::ifstream& ar, T* location, size_t count) {
		verify_type<T>();
		ar.read(reinterpret_cast<char*>(location), count * sizeof(T));
		return !ar.fail();
	}

	template<class T, class...>
	auto read_bytes(augs::stream& ar, T* location, size_t count) {
		verify_type<T>();
		return ar.read(reinterpret_cast<char*>(location), count * sizeof(T));
	}

	template<class T, class...>
	void write_bytes(augs::stream& ar, const T* location, size_t count) {
		verify_type<T>();
		ar.write(reinterpret_cast<const char*>(location), count * sizeof(T));
	}

	template<class A, class T, class...>
	auto read_object(A& ar, T& storage) {
		return read_bytes(ar, &storage, 1);
	}

	template<class A, class T, class...>
	void write_object(A& ar, const T& storage) {
		write_bytes(ar, &storage, 1);
	}

	//template<class T, class... Args>
	//void read_object(augs::stream& ar, T& storage, Args... args) {
	//	verify_type<T, std::is_same<T, augs::stream>::value>();
	//	ar.read(storage, args...);
	//}
	//
	//template<class T, class... Args>
	//void write_object(augs::stream& ar, const T& storage, Args... args) {
	//	verify_type<T, std::is_same<T, augs::stream>::value>();
	//	ar.write(storage, args...);
	//}

	template<class... Args>
	void write_object(augs::stream& ar, const augs::stream& storage, Args... args) {
		ar.write(storage, args...);
	}

	template<class A, class... Args>
	void write_stream_with_properties(A& ar, const augs::stream& storage, Args... args) {
		storage.write_with_properties(ar);
	}

	template<class A, class... Args>
	auto read_stream_with_properties(A& ar, augs::stream& storage, Args... args) {
		return storage.read_with_properties(ar);
	}

	template<class A, class... Args>
	void write_sized_stream(A& ar, const augs::stream& storage, Args... args) {
		write_object(ar, storage.buf);
	}

	template<class A, class... Args>
	auto read_sized_stream(A& ar, augs::stream& storage, Args... args) {
		auto result = read_object(ar, storage.buf);
		storage.set_write_pos(storage.buf.size());

		return result;
	}

	template<class A, size_t count>
	auto read_flags(A& ar, std::array<bool, count>& storage) {
		static_assert(count > 0, "Can't read a null array");
		
		char compressed_storage[(count - 1) / 8 + 1];
		auto result = read_bytes(ar, compressed_storage, sizeof(compressed_storage));

		for (size_t bit = 0; bit < count; ++bit)
			storage[bit] = (compressed_storage[bit / 8] >> (bit % 8)) & 1;

		return result;
	}

	template<class A, size_t count>
	void write_flags(A& ar, const std::array<bool, count>& storage) {
		static_assert(count > 0, "Can't write a null array");

		char compressed_storage[(count - 1) / 8 + 1];
		std::memset(compressed_storage, 0, sizeof(compressed_storage));

		for (size_t bit = 0; bit < count; ++bit)
			if(storage[bit])
				compressed_storage[bit / 8] |= 1 << (bit % 8);

		write_bytes(ar, compressed_storage, sizeof(compressed_storage));
	}

	template<class A, class T, class vector_size_type = size_t>
	bool read_object(A& ar, std::vector<T>& storage, vector_size_type = vector_size_type()) {
		vector_size_type s;

		if (!read_object(ar, s))
			return false;

		storage.resize(s);

		return read_bytes(ar, storage.data(), storage.size());
	}

	template<class A, class T, class vector_size_type = size_t>
	void write_object(A& ar, const std::vector<T>& storage, vector_size_type = vector_size_type()) {
		ensure(storage.size() <= std::numeric_limits<vector_size_type>::max());

		write_object(ar, static_cast<vector_size_type>(storage.size()));
		write_bytes(ar, storage.data(), storage.size());
	}

	template<class A, class T, class vector_size_type = size_t>
	bool read_vector_of_objects(A& ar, std::vector<T>& storage, vector_size_type = vector_size_type()) {
		vector_size_type s;

		if (!read_object(ar, s))
			return false;

		storage.resize(s);
		
		for (auto& obj : storage)
			if (!read_object(ar, obj))
				return false;

		return true;
	}

	template<class A, class T, class vector_size_type = size_t>
	void write_vector_of_objects(A& ar, const std::vector<T>& storage, vector_size_type = vector_size_type()) {
		ensure(storage.size() <= std::numeric_limits<vector_size_type>::max());

		write_object(ar, static_cast<vector_size_type>(storage.size()));

		for (const auto& obj : storage)
			write_object(ar, obj);
	}

	template<class A, class T, class...>
	bool read_with_capacity(A& ar, std::vector<T>& storage) {
		size_t c;
		size_t s;

		if(!read_object(ar, c)) return false;
		if(!read_object(ar, s)) return false;

		storage.reserve(c);
		storage.resize(s);

		return read_bytes(ar, storage.data(), storage.size());
	}

	template<class A, class T, class...>
	void write_with_capacity(A& ar, const std::vector<T>& storage) {
		write_object(ar, storage.capacity());
		write_object(ar, storage.size());

		write_bytes(ar, storage.data(), storage.size());
	}

	template<class A, class T, class...>
	auto read_object(A& ar, basic_pool<T>& storage) {
		return storage.read_object(ar);
	}

	template<class A, class T, class...>
	void write_object(A& ar, const basic_pool<T>& storage) {
		storage.write_object(ar);
	}

	template<class A, class T, class...>
	auto read_object(A& ar, pool<T>& storage) {
		return storage.read_object(ar);
	}

	template<class A, class T, class...>
	void write_object(A& ar, const pool<T>& storage) {
		storage.write_object(ar);
	}

	template<class A, class T, class... Args>
	auto read_object(A& ar, pool_with_meta<T, Args...>& storage) {
		return storage.read_object(ar);
	}

	template<class A, class T, class... Args>
	void write_object(A& ar, const pool_with_meta<T, Args...>& storage) {
		storage.write_object(ar);
	}

	template<class A, class T, class... Args>
	bool read_object(A& ar, std::tuple<T, Args...>& storage) {
		bool result = true;
		
		for_each_in_tuple(storage, [&ar, &result](auto& element) {
			if (!result) return;
			result = result && read_object(ar, element);
		});

		return result;
	}

	template<class A, class T, class... Args>
	void write_object(A& ar, const std::tuple<T, Args...>& storage) {
		for_each_in_tuple(storage, [&ar](const auto& element) {
			write_object(ar, element);
		});
	}

	// don't use this - we need to check for the result of reading
	// template<class T, class A>
	// T read(A& ar) {
	// 	T obj;
	// 	ensure(read_object(ar, obj));
	// 	return std::move(obj);
	// }
}