#pragma once
#include "augs/templates/memcpy_safety.h"
#include "augs/ensure.h"
#include "augs/misc/introspect.h"
#include <map>
#include <unordered_map>

namespace augs {
	template<class T>
	void verify_type() {
		static_assert(is_memcpy_safe<T>::value, "Attempt to serialize a non-trivially copyable type");
	}

	template<class A, class T, class...>
	void read_bytes(A& ar, T* const location, const size_t count) {
		verify_type<T>();
		ar.read(reinterpret_cast<char*>(location), count * sizeof(T));
	}

	template<class A, class T, class...>
	void write_bytes(A& ar, const T* const location, const size_t count) {
		verify_type<T>();
		ar.write(reinterpret_cast<const char*>(location), count * sizeof(T));
	}

	template <class T, class = void>
	struct trivial_or_introspect {
		template<class A>
		static void read(
			A& ar,
			T* const storage,
			const size_t count
		) {
			read_bytes(ar, storage, count);
		}

		template<class A>
		static void write(
			A& ar,
			const T* const storage,
			const size_t count
		) {
			write_bytes(ar, storage, count);
		}
	};

	template <class T>
	struct trivial_or_introspect<T, std::enable_if_t<has_introspects<T>::value>> {
		template<class A>
		static void read(
			A& ar,
			T* const storage,
			const size_t count
		) {
			for (size_t i = 0; i < count; ++i) {
				augs::introspect(
					storage[i],
					[&](auto& member) {
						read_object(ar, member);
					}
				);
			}
		}

		template<class A>
		static void write(
			A& ar,
			const T* const storage,
			const size_t count
		) {
			for (size_t i = 0; i < count; ++i) {
				augs::introspect(
					storage[i],
					[&](const auto& member) {
						write_object(ar, member);
					}
				);
			}
		}
	};

	template<class A, class T, class...>
	void read_object(
		A& ar, 
		T& storage
	) {
		trivial_or_introspect<T>::read(ar, &storage, 1);
	}

	template<class A, class T, class...>
	void write_object(
		A& ar, 
		const T& storage
	) {
		trivial_or_introspect<T>::write(ar, &storage, 1);
	}

	template<class A, class T, class vector_size_type = size_t>
	void read_object(
		A& ar, 
		std::vector<T>& storage, 
		vector_size_type = vector_size_type()
	) {
		vector_size_type s;
		read_object(ar, s);

		storage.resize(s);
		trivial_or_introspect<T>::read(ar, storage.data(), storage.size());
	}

	template<class A, class T, class vector_size_type = size_t>
	void write_object(
		A& ar, 
		const std::vector<T>& storage, 
		vector_size_type = vector_size_type()
	) {
		ensure(storage.size() <= std::numeric_limits<vector_size_type>::max());

		write_object(ar, static_cast<vector_size_type>(storage.size()));
		trivial_or_introspect<T>::write(ar, storage.data(), storage.size());
	}

	template <class T>
	void reserve_num_elements(T& container, const size_t n) {
		container.reserve(n);
	}

	template <class Key, class Value>
	void reserve_num_elements(std::map<Key, Value>& container, const size_t) {
	
	}

	template<class A, template <class...> class Container, class Key, class Value, class map_size_type = size_t>
	void read_associative_container(
		A& ar,
		Container<Key, Value>& storage,
		map_size_type = map_size_type()
	) {
		map_size_type s;

		read_object(ar, s);

		reserve_num_elements(storage, s);

		while (s--) {
			Key key;

			augs::read_object(ar, key);
			augs::read_object(ar, storage[key]);
		}
	}

	template<class A, template <class...> class Container, class Key, class Value, class map_size_type = size_t>
	auto write_associative_container(
		A& ar,
		const Container<Key, Value>& storage,
		map_size_type = map_size_type()
	) {
		ensure(storage.size() <= std::numeric_limits<map_size_type>::max());

		write_object(ar, static_cast<map_size_type>(storage.size()));

		for (const auto& obj : storage) {
			write_object(ar, obj.first);
			write_object(ar, obj.second);
		}
	}

	template<class A, class Key, class Value, class map_size_type = size_t>
	void write_object(
		A& ar,
		const std::map<Key, Value>& storage,
		map_size_type = map_size_type()
	) {
		write_associative_container<A, std::map, Key, Value, map_size_type>(ar, storage);
	}

	template<class A, class Key, class Value, class map_size_type = size_t>
	void read_object(
		A& ar,
		std::map<Key, Value>& storage,
		map_size_type = map_size_type()
	) {
		return read_associative_container<A, std::map, Key, Value, map_size_type>(ar, storage);
	}

	template<class A, class Key, class Value, class map_size_type = size_t>
	void write_object(
		A& ar,
		const std::unordered_map<Key, Value>& storage,
		map_size_type = map_size_type()
	) {
		write_associative_container<A, std::unordered_map, Key, Value, map_size_type>(ar, storage);
	}

	template<class A, class Key, class Value, class map_size_type = size_t>
	void read_object(
		A& ar,
		std::unordered_map<Key, Value>& storage,
		map_size_type = map_size_type()
	) {
		return read_associative_container<A, std::unordered_map, Key, Value, map_size_type>(ar, storage);
	}

	template<class A, class string_element_type, class string_size_type = size_t>
	void read_object(A& ar, std::basic_string<string_element_type>& storage, string_size_type = string_size_type()) {
		string_size_type s;

		read_object(ar, s);

		storage.resize(s);

		return read_bytes(ar, &storage[0], storage.size());
	}

	template<class A, class string_element_type, class string_size_type = size_t>
	void write_object(A& ar, const std::basic_string<string_element_type>& storage, string_size_type = string_size_type()) {
		ensure(storage.size() <= std::numeric_limits<string_size_type>::max());

		write_object(ar, static_cast<string_size_type>(storage.size()));
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

	template<class A, class T, class... Args>
	void read_object(A& ar, std::tuple<T, Args...>& storage) {
		bool result = true;
		
		for_each_in_tuple(storage, [&ar, &result](auto& element) {
			if (!result) {
				return;
			}

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

	template<class A, size_t count>
	void read_flags(A& ar, std::array<bool, count>& storage) {
		static_assert(count > 0, "Can't read a null array");

		char compressed_storage[(count - 1) / 8 + 1];
		read_bytes(ar, compressed_storage, sizeof(compressed_storage));

		for (size_t bit = 0; bit < count; ++bit) {
			storage[bit] = (compressed_storage[bit / 8] >> (bit % 8)) & 1;
		}
	}

	template<class A, size_t count>
	void write_flags(A& ar, const std::array<bool, count>& storage) {
		static_assert(count > 0, "Can't write a null array");

		char compressed_storage[(count - 1) / 8 + 1];
		std::memset(compressed_storage, 0, sizeof(compressed_storage));

		for (size_t bit = 0; bit < count; ++bit) {
			if (storage[bit]) {
				compressed_storage[bit / 8] |= 1 << (bit % 8);
			}
		}

		write_bytes(ar, compressed_storage, sizeof(compressed_storage));
	}
}