#pragma once
#include "augs/templates/memcpy_safety.h"
#include "augs/templates/find_matching_type.h"
#include "augs/ensure.h"
#include "augs/misc/introspect.h"
#include <map>
#include <unordered_map>

namespace augs {
	class output_stream_reserver;
	class stream;

	template <class Archive>
	struct is_native_binary_stream 
		: std::bool_constant<
			has_found_type_in_v<
				Archive,
				augs::stream, 
				augs::output_stream_reserver, 
				std::ifstream, 
				std::ofstream
			>
		>
	{

	};

	template <class Archive, class Serialized, class = void>
	struct has_io_overloads 
		: std::false_type 
	{

	};

	template <class Archive, class Serialized>
	struct has_io_overloads <
		Archive,
		Serialized,
		decltype(
			read_object(
				std::declval<Archive>(),
				std::declval<Serialized>()
			),
			write_object(
				std::declval<Archive>(),
				std::declval<const Serialized>()
			),
			void()
		)
	> : std::true_type {
	
	};

	template <class Archive, class Serialized>
	constexpr bool has_io_overloads_v = has_io_overloads<Archive, Serialized>::value;

	template<class Archive, class Serialized>
	struct is_byte_io_safe 
		: std::bool_constant<is_native_binary_stream<Archive>::value && is_memcpy_safe_v<Serialized>> 
	{

	};

	template<class Archive, class Serialized>
	constexpr bool is_byte_io_safe_v = is_byte_io_safe<Archive, Serialized>::value;

	template<class Archive, class Serialized>
	struct is_byte_io_appropriate 
		: std::bool_constant<is_byte_io_safe_v<Archive, Serialized> && !has_io_overloads_v<Archive, Serialized>>
	{

	};

	template<class Archive, class Serialized>
	constexpr bool is_byte_io_appropriate_v = is_byte_io_appropriate<Archive, Serialized>::value;

	template<class Archive, class Serialized>
	void verify_byte_io_safety() {
		static_assert(is_memcpy_safe_v<Serialized>, "Attempt to serialize a non-trivially copyable type");
		static_assert(is_native_binary_stream<Archive>::value, "Byte serialization of trivial structs allowed only on native binary archives");
	}

	template<class Archive, class Serialized>
	void read_bytes(
		Archive& ar, 
		Serialized* const location, 
		const size_t object_count
	) {
		verify_byte_io_safety<Archive, Serialized>();
		ar.read(reinterpret_cast<char*>(location), object_count * sizeof(Serialized));
	}

	template<class Archive, class Serialized>
	void write_bytes(
		Archive& ar, 
		const Serialized* const location, 
		const size_t object_count
	) {
		verify_byte_io_safety<Archive, Serialized>();
		ar.write(reinterpret_cast<const char*>(location), object_count * sizeof(Serialized));
	}

	template<class Archive, class Serialized>
	void read(
		Archive& ar,
		Serialized& storage,
		const std::enable_if_t<has_io_overloads_v<Archive, Serialized>>* const dummy = nullptr
	) {
		read_object(ar, storage);
	}

	template<class Archive, class Serialized>
	void write(
		Archive& ar,
		const Serialized& storage,
		const std::enable_if_t<has_io_overloads_v<Archive, Serialized>>* const dummy = nullptr
	) {
		write_object(ar, storage);
	}

	template<class Archive, class Serialized>
	void read(
		Archive& ar,
		Serialized& storage,
		const std::enable_if_t<is_byte_io_appropriate_v<Archive, Serialized>>* const dummy = nullptr
	) {
		read_bytes(ar, &storage, 1);
	}

	template<class Archive, class Serialized>
	void write(
		Archive& ar,
		const Serialized& storage,
		const std::enable_if_t<is_byte_io_appropriate_v<Archive, Serialized>>* const dummy = nullptr
	) {
		write_bytes(ar, &storage, 1);
	}

	template<class Archive, class Serialized>
	void read(
		Archive& ar,
		Serialized& storage,
		const std::enable_if_t<!has_io_overloads_v<Archive, Serialized> && !is_byte_io_safe_v<Archive, Serialized>>* const dummy = nullptr
	) {
		static_assert(has_introspect_v<Serialized>, "Attempt to read a type in a non-bytesafe context without i/o overloads and without introspectors!");

		augs::introspect(
			[&](auto, auto& member) {
				read(ar, member);
			},
			storage
		);
	}

	template<class Archive, class Serialized>
	void write(
		Archive& ar,
		const Serialized& storage,
		const std::enable_if_t<!has_io_overloads_v<Archive, Serialized> && !is_byte_io_safe_v<Archive, Serialized>>* const dummy = nullptr
	) {
		static_assert(has_introspect_v<Serialized>, "Attempt to write a type in a non-bytesafe context without i/o overloads and without introspectors!");

		augs::introspect(
			[&](auto, const auto& member) {
				write(ar, member);
			},
			storage
		);
	}

	template<class Archive, class Serialized>
	void read_n(
		Archive& ar,
		Serialized* const storage,
		const size_t n,
		const std::enable_if_t<is_byte_io_appropriate_v<Archive, Serialized>>* const dummy = nullptr
	) {
		read_bytes(ar, storage, n);
	}

	template<class Archive, class Serialized>
	void write_n(
		Archive& ar,
		const Serialized* const storage,
		const size_t n,
		const std::enable_if_t<is_byte_io_appropriate_v<Archive, Serialized>>* const dummy = nullptr
	) {
		write_bytes(ar, storage, n);
	}

	template<class Archive, class Serialized>
	void read_n(
		Archive& ar,
		Serialized* const storage,
		const size_t n,
		const std::enable_if_t<!is_byte_io_appropriate_v<Archive, Serialized>>* const dummy = nullptr
	) {
		for (size_t i = 0; i < n; ++i) {
			read(ar, storage[i]);
		}
	}

	template<class Archive, class Serialized>
	void write_n(
		Archive& ar,
		const Serialized* const storage,
		const size_t n,
		const std::enable_if_t<!is_byte_io_appropriate_v<Archive, Serialized>>* const dummy = nullptr
	) {
		for (size_t i = 0; i < n; ++i) {
			write(ar, storage[i]);
		}
	}
	
	template <class Archive, class Serialized, class vector_size_type = size_t>
	void read_object(
		Archive& ar, 
		std::vector<Serialized>& storage, 
		vector_size_type = vector_size_type()
	) {
		vector_size_type s;
		read(ar, s);

		storage.resize(s);
		read_n(ar, storage.data(), storage.size());
	}

	template<class Archive, class Serialized, class vector_size_type = size_t>
	void write_object(
		Archive& ar, 
		const std::vector<Serialized>& storage, 
		vector_size_type = vector_size_type()
	) {
		ensure(storage.size() <= std::numeric_limits<vector_size_type>::max());

		write(ar, static_cast<vector_size_type>(storage.size()));
		write_n(ar, storage.data(), storage.size());
	}

	template <class Serialized>
	void reserve_num_elements(Serialized& container, const size_t n) {
		container.reserve(n);
	}

	template <class Key, class Value>
	void reserve_num_elements(std::map<Key, Value>& container, const size_t) {
	
	}

	template<class Archive, template <class...> class Container, class Key, class Value, class map_size_type = size_t>
	void read_associative_container(
		Archive& ar,
		Container<Key, Value>& storage,
		map_size_type = map_size_type()
	) {
		map_size_type s;

		read(ar, s);

		reserve_num_elements(storage, s);

		while (s--) {
			Key key;

			read(ar, key);
			read(ar, storage[key]);
		}
	}

	template<class Archive, template <class...> class Container, class Key, class Value, class map_size_type = size_t>
	auto write_associative_container(
		Archive& ar,
		const Container<Key, Value>& storage,
		map_size_type = map_size_type()
	) {
		ensure(storage.size() <= std::numeric_limits<map_size_type>::max());

		write(ar, static_cast<map_size_type>(storage.size()));

		for (const auto& obj : storage) {
			write(ar, obj.first);
			write(ar, obj.second);
		}
	}

	template<class Archive, class Key, class Value, class map_size_type = size_t>
	void write_object(
		Archive& ar,
		const std::map<Key, Value>& storage,
		map_size_type = map_size_type()
	) {
		write_associative_container<Archive, std::map, Key, Value, map_size_type>(ar, storage);
	}

	template<class Archive, class Key, class Value, class map_size_type = size_t>
	void read_object(
		Archive& ar,
		std::map<Key, Value>& storage,
		map_size_type = map_size_type()
	) {
		read_associative_container<Archive, std::map, Key, Value, map_size_type>(ar, storage);
	}

	template<class Archive, class Key, class Value, class map_size_type = size_t>
	void write_object(
		Archive& ar,
		const std::unordered_map<Key, Value>& storage,
		map_size_type = map_size_type()
	) {
		write_associative_container<Archive, std::unordered_map, Key, Value, map_size_type>(ar, storage);
	}

	template<class Archive, class Key, class Value, class map_size_type = size_t>
	void read_object(
		Archive& ar,
		std::unordered_map<Key, Value>& storage,
		map_size_type = map_size_type()
	) {
		read_associative_container<Archive, std::unordered_map, Key, Value, map_size_type>(ar, storage);
	}

	template<class Archive, class string_element_type, class string_size_type = size_t>
	void read_object(Archive& ar, std::basic_string<string_element_type>& storage, string_size_type = string_size_type()) {
		string_size_type s;

		read(ar, s);

		storage.resize(s);

		read_n(ar, &storage[0], storage.size());
	}

	template<class Archive, class string_element_type, class string_size_type = size_t>
	void write_object(Archive& ar, const std::basic_string<string_element_type>& storage, string_size_type = string_size_type()) {
		ensure(storage.size() <= std::numeric_limits<string_size_type>::max());

		write(ar, static_cast<string_size_type>(storage.size()));
		write_n(ar, &storage[0], storage.size());
	}

	template<class Archive, class Serialized, class...>
	void read_with_capacity(Archive& ar, std::vector<Serialized>& storage) {
		size_t c;
		size_t s;

		read(ar, c);
		read(ar, s);

		storage.reserve(c);
		storage.resize(s);

		read_n(ar, storage.data(), storage.size());
	}

	template<class Archive, class Serialized>
	void write_with_capacity(Archive& ar, const std::vector<Serialized>& storage) {
		write(ar, storage.capacity());
		write(ar, storage.size());

		write_n(ar, storage.data(), storage.size());
	}

	template<class Archive, class Serialized, class... Args>
	void read_object(Archive& ar, std::tuple<Serialized, Args...>& storage) {
		for_each_in_tuple(storage, [&](auto& element) {
			read(ar, element);
		});
	}

	template<class Archive, class Serialized, size_t N>
	void read_object(Archive& ar, std::array<Serialized, N>& storage) {
		read_n(ar, storage.data(), storage.size());
	}

	template<class Archive, class Serialized, size_t N>
	void write_object(Archive& ar, const std::array<Serialized, N>& storage) {
		write_n(ar, storage.data(), storage.size());
	}

	template<class Archive, class Serialized, class... Args>
	void write_object(Archive& ar, const std::tuple<Serialized, Args...>& storage) {
		for_each_in_tuple(storage, [&](const auto& element) {
			write(ar, element);
		});
	}

	template<class Archive, size_t count>
	void read_flags(Archive& ar, std::array<bool, count>& storage) {
		static_assert(count > 0, "Can't read a null array");

		std::array<char, (count - 1) / 8 + 1> compressed_storage;
		read(ar, compressed_storage);

		for (size_t bit = 0; bit < count; ++bit) {
			storage[bit] = (compressed_storage[bit / 8] >> (bit % 8)) & 1;
		}
	}

	template<class Archive, size_t count>
	void write_flags(Archive& ar, const std::array<bool, count>& storage) {
		static_assert(count > 0, "Can't write a null array");

		std::array<char, (count - 1) / 8 + 1> compressed_storage;
		std::fill(compressed_storage.begin(), compressed_storage.end(), 0);

		for (size_t bit = 0; bit < count; ++bit) {
			if (storage[bit]) {
				compressed_storage[bit / 8] |= 1 << (bit % 8);
			}
		}

		write(ar, compressed_storage);
	}
}