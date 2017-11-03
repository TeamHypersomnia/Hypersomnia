#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <chrono>
#include <algorithm>

#include <fstream>
#include <experimental/filesystem>

#include "augs/ensure.h"
#include "augs/templates/byte_type_for.h"
#include "augs/filesystem/path.h"
#include "augs/readwrite/byte_readwrite_declaration.h"

namespace augs {
	template <class T, class... Args>
	auto with_exceptions(Args&&... args) {
		auto stream = T(std::forward<Args>(args)...);
		stream.exceptions(T::failbit | T::badbit);
		return stream;
	}

	inline auto open_binary_output_stream(const augs::path_type& path) {
		auto s = with_exceptions<std::ofstream>();
		s.open(path, std::ios::out | std::ios::binary);
		return s;
	}

	inline auto open_binary_input_stream(const augs::path_type& path) {
		auto s = with_exceptions<std::ifstream>();
		s.open(path, std::ios::in | std::ios::binary);
		return s;
	}

	using ifstream_error = std::ifstream::failure;
	
	inline std::chrono::system_clock::time_point last_write_time(const path_type& path) {
		return std::experimental::filesystem::last_write_time(path);
	}

	inline bool file_exists(const path_type& path) {
		return std::ifstream(path).good();
	}

	inline void remove_file(const path_type& path) {
		std::experimental::filesystem::remove(path);
	}

	inline path_type first_free_path(const path_type path_template) {
		for (std::size_t candidate = 0;; ++candidate) {
			const auto candidate_path = candidate ? 
				typesafe_sprintf(path_template.string(), typesafe_sprintf("-%x", candidate))
				: typesafe_sprintf(path_template.string(), "")
			;

			if (!file_exists(candidate_path)) {
				return candidate_path;
			}
		}
	}

	inline path_type switch_path(
		const path_type canon_path,
		const path_type local_path
	) {
		if (file_exists(local_path)) {
			return local_path;
		}
		else {
			return canon_path;
		}
	}

	template <class C = char>
	auto file_to_string(const path_type& path, const C = C()) {
		auto t = with_exceptions<std::basic_ifstream<C>>();
		t.open(path);

		std::basic_stringstream<C> buffer;
		buffer << t.rdbuf();

		return buffer.str();
	}

	inline auto file_to_bytes(const path_type& path) {
		auto file = with_exceptions<std::ifstream>();
		file.open(path, std::ios::binary | std::ios::ate);

		const std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<std::byte> output;

		output.resize(size);
		file.read(reinterpret_cast<byte_type_for_t<std::ifstream>*>(output.data()), size);
		return output;
	}

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

	template <class S>
	void save_as_text(const path_type& path, const S& text) {
		auto out = with_exceptions<std::ofstream>();
		out.open(path, std::ios::out);
		out << text;
	}

	template <class S>
	void save_as_text_if_different(const path_type& path, const S& text) {
		if (!file_exists(path) || text != file_to_string(path)) {
			auto out = with_exceptions<std::ofstream>();
			out.open(path, std::ios::out);
			out << text;
		}
	}

	template <class O>
	void save_as_bytes(const O& object, const path_type& path) {
		auto out = open_binary_output_stream(path);
		write_bytes(out, object);
	}

	inline void save_as_bytes(const std::vector<std::byte>& bytes, const path_type& path) {
		auto out = open_binary_output_stream(path);
		out.write(reinterpret_cast<const byte_type_for_t<decltype(out)>*>(bytes.data()), bytes.size());
	}

	template <class ContainerType>
	void read_map_until_eof(const path_type& path, ContainerType& into) {
		auto source = open_binary_input_stream(path);

		while (source.peek() != EOF) {
			typename ContainerType::key_type key{};
			typename ContainerType::mapped_type value{};

			augs::read_bytes(source, key);
			augs::read_bytes(source, value);

			into.emplace(std::move(key), std::move(value));
		}
	}
}