#pragma once
#include <cstddef>

namespace augs {
	namespace detail {
		template <class Archive, class Serialized>
		void read_raw_bytes(Archive& ar, Serialized* const location, const std::size_t object_count);

		template <class Archive, class Serialized>
		void write_raw_bytes(Archive& ar, const Serialized* const location, const std::size_t object_count);
	}

	template <class Archive, class Serialized>
	void read_bytes(Archive&, Serialized&);

	template <class Archive, class Serialized>
	void write_bytes(Archive&, const Serialized&);

	template <class Archive, class Container, class container_size_type = std::size_t>
	void read_container(
		Archive& ar,
		Container& storage,
		container_size_type = container_size_type()
	);

	template <class Archive, class Container, class container_size_type = std::size_t>
	void write_container(
		Archive& ar,
		const Container& storage,
		container_size_type = {}
	);

	template <class Archive, class Container, class container_size_type = std::size_t>
	void read_capacity(
		Archive& ar,
		Container& storage,
		container_size_type = {}
	);

	template<class Archive, class Container, class container_size_type = std::size_t>
	void write_capacity(Archive& ar, const Container& storage);
}