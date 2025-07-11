#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

#include "augs/ensure.h"
#include "augs/pad_bytes.h"

#include "augs/templates/traits/container_traits.h"
#include "augs/templates/byte_type_for.h"
#include "augs/misc/enum/is_enum_boolset.h"
#include "augs/templates/traits/is_variant.h"
#include "augs/templates/traits/is_monostate.h"
#include "augs/templates/traits/is_optional.h"
#include "augs/templates/traits/is_unique_ptr.h"
#include "augs/templates/traits/is_std_array.h"
#include "augs/templates/for_each_type.h"
#include "augs/templates/introspect.h"
#include "augs/templates/traits/introspection_traits.h"

#include "augs/readwrite/byte_readwrite_declaration.h"
#include "augs/readwrite/byte_readwrite_overload_traits.h"
#include "augs/readwrite/byte_readwrite_traits.h"
#include "augs/readwrite/sane_max_size.h"
#include "augs/readwrite/stream_read_error.h"
#include "augs/templates/resize_no_init.h"

#if DEBUG_DESYNCS
#include "augs/string/get_type_name.h"
#include "augs/log.h"

extern bool LOG_BYTE_SERIALIZE;
#endif

#ifdef write_bytes
#undef write_bytes
#endif

#ifdef read_bytes
#undef read_bytes
#endif

namespace augs {
	namespace detail {	
		template <class Archive, class Serialized>
		void read_raw_bytes(
			Archive& ar, 
			Serialized* const location, 
			const std::size_t object_count
		) {
			verify_byte_readwrite_safety<Archive, Serialized>();

			const auto byte_count = object_count * sizeof(Serialized);
			auto* const byte_location = reinterpret_cast<byte_type_for_t<Archive>*>(location);
			
			ar.read(byte_location, byte_count);
		}

		template <class Archive, class Serialized>
		void write_raw_bytes(
			Archive& ar, 
			const Serialized* const location, 
			const std::size_t object_count
		) {
			verify_byte_readwrite_safety<Archive, Serialized>();
			
#if DEBUG_DESYNCS
			if constexpr(std::is_same_v<Archive, std::ofstream>) {
				if (LOG_BYTE_SERIALIZE) {
					if constexpr(can_stream_left_v<std::ostringstream, Serialized>) {
						LOG_NVPS(ar.tellp(), get_type_name<Serialized>(), object_count, *location);
					}
					else {
						LOG_NVPS(ar.tellp(), get_type_name<Serialized>(), object_count);
					}
				}
			}
#endif

			const auto byte_count = object_count * sizeof(Serialized);
			const auto* const byte_location = reinterpret_cast<const byte_type_for_t<Archive>*>(location);

			ar.write(byte_location, byte_count);
		}

		template <class Archive, class Serialized>
		void read_bytes_n(
			Archive& ar,
			Serialized* const storage,
			const std::size_t n
		) {
			if constexpr(is_byte_readwrite_appropriate_v<Archive, Serialized>) {
				detail::read_raw_bytes(ar, storage, n);
			}
			else {
				for (std::size_t i = 0; i < n; ++i) {
					augs::read_bytes(ar, storage[i]);
				}
			}
		}
	}

	template <class Archive, class Serialized>
	void verify_read_bytes() {
		static_assert(is_byte_stream_v<Archive>, "Trying to read from a non-byte stream.");
		static_assert(!std::is_const_v<Serialized>, "Trying to read bytes to a const object.");
		static_assert(!std::is_same_v<Serialized, std::nullptr_t> && !std::is_same_v<Serialized, std::nullopt_t> , "Trying to read bytes to a null object.");
	}

	template <class Archive, class Serialized>
	void read_bytes_no_overload(Archive& ar, Serialized& storage) {
		verify_read_bytes<Archive, Serialized>();

		if constexpr(is_unique_ptr_v<Serialized>) {
			bool has_value = false;
			read_bytes(ar, has_value);

			if (has_value) {
				if (storage == nullptr) {
					storage = std::make_unique<typename Serialized::element_type>();
				}

				read_bytes(ar, *storage);
			}
			else {
				storage.release();
			}
		}
		else if constexpr(is_optional_v<Serialized>) {
			bool has_value = false;
			read_bytes(ar, has_value);

			if (has_value) {
				if (storage == std::nullopt) {
					storage.emplace();
				}

				read_bytes(ar, *storage);
			}
			else {
				storage.reset();
			}
		}
		else if constexpr(is_variant_v<Serialized>) {
			auto type_id = static_cast<uint8_t>(-1);
			read_bytes(ar, type_id);

			if (type_id != static_cast<uint8_t>(-1)) {
				for_each_type_in_list<Serialized>(
					[&](const auto& dummy) {
						using T = remove_cref<decltype(dummy)>;

						if (type_id == index_in_list_v<T, Serialized>) {
							T object;

							if constexpr(!is_monostate_v<decltype(object)>) {
								read_bytes(ar, object);
							}

							storage.template emplace<T>(std::move(object));
						}
					}
				);
			}
			else {
				storage = {};
			}
		}
		else if constexpr(is_std_array_v<Serialized> || is_enum_array_v<Serialized>) {
			detail::read_bytes_n(ar, storage.data(), storage.size());
		}
		else if constexpr(is_enum_boolset_v<Serialized>) {
			detail::read_raw_bytes(ar, &storage, 1);
		}
		else if constexpr(is_container_v<Serialized>) {
			read_container_bytes(ar, storage);
		}
		else if constexpr(is_byte_readwrite_appropriate_v<Archive, Serialized>) {
			detail::read_raw_bytes(ar, &storage, 1);
		}
		else {
			verify_has_introspect(storage);

			introspect(
				[&](auto, auto& member) {
					using T = remove_cref<decltype(member)>;
					
					if constexpr (!is_padding_field_v<T>) {
						read_bytes(ar, member);
					}
				},
				storage
			);
		}
	}

	template <class Archive, class Serialized>
	void read_bytes(Archive& ar, Serialized& storage) {
		verify_read_bytes<Archive, Serialized>();

		if constexpr(has_special_read_v<Archive, Serialized>) {
			ar.special_read(storage);
		}
		else if constexpr(has_byte_read_overload_v<Archive, Serialized>) {
			static_assert(has_byte_write_overload_v<Archive, Serialized>, "Has read_object_bytes overload, but no write_object_bytes overload.");

			read_object_bytes(ar, storage);
		}
		else {
			read_bytes_no_overload(ar, storage);
		}
	}

	template <class Serialized, class Archive>
	Serialized read_bytes(Archive& ar) {
		Serialized storage;
		augs::read_bytes(ar, storage);
		return storage;
	}

	namespace detail {
		template <class Archive, class Serialized>
		void write_bytes_n(
			Archive& ar,
			const Serialized* const storage,
			const std::size_t n
		) {
			if constexpr(is_byte_readwrite_appropriate_v<Archive, Serialized>) {
				detail::write_raw_bytes(ar, storage, n);
			}
			else {
				for (std::size_t i = 0; i < n; ++i) {
					augs::write_bytes(ar, storage[i]);
				}
			}
		}
	}

	
	template <class Archive, class Serialized>
	void verify_write_bytes() {
		static_assert(is_byte_stream_v<Archive>, "Trying to write to a non-byte stream.");
		static_assert(!std::is_same_v<Serialized, std::nullptr_t> && !std::is_same_v<Serialized, std::nullopt_t> , "Trying to write bytes from a null object.");
	}

	template <class Archive, class Serialized>
	void write_bytes_no_overload(Archive& ar, const Serialized& storage) {
		verify_write_bytes<Archive, Serialized>();

		if constexpr(is_optional_v<Serialized>) {
			write_bytes(ar, storage.has_value());

			if (storage) {
				write_bytes(ar, *storage);
			}
		}
		else if constexpr(is_unique_ptr_v<Serialized>) {
			write_bytes(ar, storage != nullptr);

			if (storage) {
				write_bytes(ar, *storage);
			}
		}
		else if constexpr(is_variant_v<Serialized>) {
			const auto index = storage.index();

			static_assert(num_types_in_list_v<Serialized> < static_cast<uint8_t>(-1));

			if (index == std::variant_npos) {
				write_bytes(ar, static_cast<uint8_t>(-1));
			}
			else {
				write_bytes(ar, static_cast<uint8_t>(index));

				std::visit(
					[&](const auto& object) {
						if constexpr(!is_monostate_v<decltype(object)>) {
							write_bytes(ar, object);
						}
					}, 
					storage
				);
			}
		}
		else if constexpr(is_std_array_v<Serialized> || is_enum_array_v<Serialized>) {
			detail::write_bytes_n(ar, storage.data(), storage.size());
		}
		else if constexpr(is_enum_boolset_v<Serialized>) {
			detail::write_raw_bytes(ar, &storage, 1);
		}
		else if constexpr(is_container_v<Serialized>) {
			write_container_bytes(ar, storage);
		}
		else if constexpr(is_byte_readwrite_appropriate_v<Archive, Serialized>) {
			detail::write_raw_bytes(ar, &storage, 1);
		}
		else {
			verify_has_introspect(storage);

			introspect(
				[&](auto, const auto& member) {
					using T = remove_cref<decltype(member)>;

					if constexpr (!is_padding_field_v<T>) {
						write_bytes(ar, member);
					}
				},
				storage
			);
		}
	}

	template <class Archive, class Serialized>
	void write_bytes(Archive& ar, const Serialized& storage) {
#if !PLATFORM_WEB
		static_assert(!std::is_same_v<Serialized, std::size_t>, "Don't serialize size_t, it will cause incompatbility with the Web!");
#endif
		if constexpr(has_special_write_v<Archive, Serialized>) {
			ar.special_write(storage);
		}
		else if constexpr(has_byte_write_overload_v<Archive, Serialized>) {
			static_assert(has_byte_read_overload_v<Archive, remove_cref<Serialized>&>, "Has write_object_bytes overload, but no read_object_bytes overload.");

			write_object_bytes(ar, storage);
		}
		else {
			write_bytes_no_overload(ar, storage);
		}
	}

	template <class Archive, class Container, class container_size_type>
	void read_container_bytes(
		Archive& ar, 
		Container& storage, 
		container_size_type
	) {
		storage.clear();

		container_size_type s;
		read_bytes(ar, s);

		if (s == 0) {
			return;
		}

		if (s > sane_max_size_map::at<Container>) {
			throw stream_read_error(
				"Requested storage size is bigger than the sane max size!"
			);
		}

		if (s > storage.max_size()) {
			throw stream_read_error(
				"Requested storage size is bigger than its max_size!"
			);
		}

		if constexpr(can_access_data_v<Container>) {
			resize_no_init(storage, s);
			detail::read_bytes_n(ar, storage.data(), s);
		}
		else {
			if constexpr(can_reserve_v<Container>) {
				storage.reserve(s);
			}

			if constexpr(is_associative_v<Container>) {
				while (s--) {
					typename Container::key_type key;
					typename Container::mapped_type mapped;

					read_bytes(ar, key);
					read_bytes(ar, mapped);

					storage.emplace(std::move(key), std::move(mapped));
				}
			}
			else {
				while (s--) {
					typename Container::value_type val;

					read_bytes(ar, val);

					storage.emplace(std::move(val));
				}
			}
		}
	}

	template <class Archive, class Container, class container_size_type>
	void write_container_bytes(
		Archive& ar, 
		const Container& storage, 
		container_size_type
	) {
		const auto s = storage.size();
		ensure(s <= std::numeric_limits<container_size_type>::max());
		ensure(s <= sane_max_size_map::at<Container>);
		write_bytes(ar, static_cast<container_size_type>(s));

		if (s == 0) {
			return;
		}

		if constexpr(can_access_data_v<Container>) {
			detail::write_bytes_n(ar, storage.data(), s);
		}
		else {
			if constexpr(is_associative_v<Container>) {
				for (auto&& it : storage) {
					write_bytes(ar, it.first);
					write_bytes(ar, it.second);
				}
			}
			else {
				for (const auto& obj : storage) {
					write_bytes(ar, obj);
				}
			}
		}
	}

	template <class Archive, class Container, class container_size_type>
	void read_capacity_bytes(
		Archive& ar, 
		Container& storage,
		container_size_type
	) {
		container_size_type c;
		read_bytes(ar, c);

		(void)storage;
	}

	template<class Archive, class Container, class container_size_type>
	void write_capacity_bytes(Archive& ar, const Container& storage) {
		const auto c = static_cast<container_size_type>(0);
		ensure(c <= std::numeric_limits<container_size_type>::max());
		write_bytes(ar, c);

		(void)storage;
	}

	template<class Archive, std::size_t count>
	void read_flags(Archive& ar, std::array<bool, count>& storage) {
		static_assert(count > 0, "Can't read_bytes a null array");

		std::array<std::byte, (count - 1) / 8 + 1> compressed_storage;
		read_bytes(ar, compressed_storage);

		for (std::size_t bit = 0; bit < count; ++bit) {
			storage[bit] = std::to_integer<int>((compressed_storage[bit / 8] >> (bit % 8)) & static_cast<std::byte>(1)) != 0;
		}
	}

	template<class Archive, std::size_t count>
	void write_flags(Archive& ar, const std::array<bool, count>& storage) {
		static_assert(count > 0, "Can't write_bytes a null array");

		std::array<std::byte, (count - 1) / 8 + 1> compressed_storage;
		
		for (auto& c : compressed_storage) {
			c = static_cast<std::byte>(0);
		}

		for (std::size_t bit = 0; bit < count; ++bit) {
			if (storage[bit]) {
				compressed_storage[bit / 8] |= static_cast<std::byte>(1 << (bit % 8));
			}
		}

		write_bytes(ar, compressed_storage);
	}
}