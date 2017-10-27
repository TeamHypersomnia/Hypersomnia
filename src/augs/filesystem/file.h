#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <chrono>
#include <algorithm>

#include <fstream>
#include <experimental/filesystem>

#include "augs/templates/byte_type_for.h"
#include "augs/filesystem/path.h"
#include "augs/readwrite/streams.h"
#include "augs/ensure.h"

namespace augs {
	template <class T, class... Args>
	auto with_exceptions(Args&&... args) {
		auto stream = T(std::forward<Args>(args)...);
		stream.exceptions(T::failbit | T::badbit);
		return stream;
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
	auto get_file_contents(const path_type& path, const C = C()) {
		auto t = with_exceptions<std::basic_ifstream<C>>();
		t.open(path);

		std::basic_stringstream<C> buffer;
		buffer << t.rdbuf();

		return buffer.str();
	}

	inline std::vector<std::string> get_file_lines_without_blanks_and_comments(
		const path_type& path,
		const char comment_begin_character = '%'
	) {
		auto input = with_exceptions<std::ifstream>();
		input.open(path);
	
		std::vector<std::string> out;
	
		for (std::string line; std::getline(input, line); ) {
			const bool should_omit = 
				std::all_of(line.begin(), line.end(), isspace) 
				|| line[0] == comment_begin_character
			;
	
			if(!should_omit) {
				out.emplace_back(line);
			}
		}
	
		return out;
	}

	inline auto get_file_lines(const path_type& path) {
		using string_type = std::string;

		auto input = with_exceptions<std::ifstream>();
		input.open(path);

		std::vector<string_type> out;

		for (string_type line; std::getline(input, line);) {
			out.push_back(line);
		}

		return out;
	}

	template <class S>
	void create_text_file(const path_type& path, const S& text) {
		auto out = with_exceptions<std::ofstream>();
		out.open(path, std::ios::out);
		out << text;
	}

	template <class S>
	void create_text_file_if_different(const path_type& path, const S& text) {
		if (!file_exists(path) || text != get_file_contents(path)) {
			auto out = with_exceptions<std::ofstream>();
			out.open(path, std::ios::out);
			out << text;
		}
	}

	template <class C>
	void create_binary_file(const path_type& path, const C& content) {
		auto out = with_exceptions<std::ofstream>();
		out.open(path, std::ios::out | std::ios::binary);
		out.write(reinterpret_cast<const byte_type_for_t<std::ofstream>*>(content.data()), content.size() * sizeof(content[0]));
	}

	template <class O>
	void save(const O& object, const path_type& path) {
		augs::stream content;
		augs::write_bytes(content, object);

		create_binary_file(path, content);
	}

	template <class S>
	void get_file_contents_binary_into(const path_type& path, S& target) {
		auto file = with_exceptions<std::ifstream>();
		file.open(path, std::ios::binary | std::ios::ate);

		const std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		target.reserve(static_cast<unsigned>(size));
		file.read(reinterpret_cast<byte_type_for_t<std::ifstream>*>(target.data()), size);
		target.set_write_pos(static_cast<size_t>(size));
	}

	template <class O>
	void load(O& object, const path_type& path) {
		augs::stream content;
		get_file_contents_binary_into(path, content);

		augs::read_bytes(content, object);
	}

	template <class O>
	O load(const path_type& path) {
		augs::stream content;
		get_file_contents_binary_into(path, content);

		O object;
		augs::read_bytes(content, object);
		return object;
	}

	template <class ContainerType>
	void read_map_until_eof(const path_type& path, ContainerType& into) {
		auto source = with_exceptions<std::ifstream>();
		source.open(path, std::ios::binary | std::ios::in);

		while (source.peek() != EOF) {
			typename ContainerType::key_type key{};
			typename ContainerType::mapped_type value{};

			augs::read_bytes(source, key);
			augs::read_bytes(source, value);

			into.emplace(std::move(key), std::move(value));
		}
	}
}