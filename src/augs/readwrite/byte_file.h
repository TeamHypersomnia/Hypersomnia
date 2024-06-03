#pragma once
#include "augs/filesystem/file.h"
#include "augs/templates/byte_type_for.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/file_to_bytes.h"
#include "augs/filesystem/path_declaration.h"

namespace augs {
	template <class O>
	void load_from_bytes(O& object, const path_type& path) {
		auto file = open_binary_input_stream(path);
		augs::read_bytes(file, object);
	}

	template <class O>
	O load_from_bytes(const path_type& path) {
		O object;
		load_from_bytes(object, path);
		return object;
	}

	template <class O>
	void save_as_bytes(const O& object, const path_type& path) {
		if constexpr(has_value_type_v<O>) {
			static_assert(!std::is_same_v<typename O::value_type, std::byte>, "Use bytes_to_file for directly writing the binary files!");
		}

		{
			auto out = open_binary_output_stream(path);
			write_bytes(out, object);
		}

		sync_if_persistent(path);
	}

	inline void save_string_as_bytes(const std::string& bytes, const path_type& path) {
		{
			auto out = open_binary_output_stream(path);
			out.write(reinterpret_cast<const byte_type_for_t<decltype(out)>*>(bytes.data()), bytes.size());
		}

		sync_if_persistent(path);
	}

	template <class S>
	inline void bytes_to_file(const S& source, const path_type& path) {
		{
			auto out = open_binary_output_stream(path);
			out.write(reinterpret_cast<const byte_type_for_t<decltype(out)>*>(source.data()), source.size());
		}

		sync_if_persistent(path);
	}

	template <class S, class ContainerType>
	void read_map_until_eof(S& source, ContainerType& into) {
		while (source.peek() != EOF) {
			typename ContainerType::key_type key;
			typename ContainerType::mapped_type value;

			augs::read_bytes(source, key);
			augs::read_bytes(source, value);

			into.emplace(std::move(key), std::move(value));
		}
	}

	template <class ContainerType>
	void read_map_until_eof(const path_type& path, ContainerType& into) {
		auto source = open_binary_input_stream(path);

		read_map_until_eof(source, into);
	}

	template <class Source, class ContainerType>
	void read_vector_until_eof(Source& source, ContainerType& into) {
		while (source.peek() != EOF) {
			typename ContainerType::value_type value;
			augs::read_bytes(source, value);
			into.emplace_back(std::move(value));
		}
	}
}
