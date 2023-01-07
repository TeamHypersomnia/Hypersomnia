#pragma once
#include <cstddef>

namespace augs {
	namespace detail {
		template <class Archive, class Serialized>
		void read_raw_bytes(Archive& ar, Serialized* const location, const std::size_t object_count);

		template <class Archive, class Serialized>
		void write_raw_bytes(Archive& ar, const Serialized* const location, const std::size_t object_count);
	}

	template <class Serialized, class Archive>
	Serialized read_bytes(Archive& ar);

	template <class Archive, class Serialized>
	void read_bytes(Archive&, Serialized&);

	template <class Archive, class Serialized>
	void write_bytes(Archive&, const Serialized&);

	template <class Archive, class Serialized>
	void read_bytes_no_overload(Archive&, Serialized&);

	template <class Archive, class Serialized>
	void write_bytes_no_overload(Archive&, const Serialized&);

	template <class Archive, class Container, class container_size_type = uint32_t>
	void read_container_bytes(
		Archive& ar,
		Container& storage,
		container_size_type = container_size_type()
	);

	template <class Archive, class Container, class container_size_type = uint32_t>
	void write_container_bytes(
		Archive& ar,
		const Container& storage,
		container_size_type = {}
	);

	template <class Archive, class Container, class container_size_type = uint32_t>
	void read_capacity_bytes(
		Archive& ar,
		Container& storage,
		container_size_type = {}
	);

	template<class Archive, class Container, class container_size_type = uint32_t>
	void write_capacity_bytes(Archive& ar, const Container& storage);
}