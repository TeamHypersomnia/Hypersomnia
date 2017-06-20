#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <chrono>

#include <fstream>
#include <experimental\filesystem>

namespace augs {
	namespace fs = std::experimental::filesystem;
	
	template <class P>
	std::chrono::system_clock::time_point last_write_time(const P& path) {
		ensure_existence(path);
		return fs::last_write_time(path);
	}

	template <class P>
	bool file_exists(const P& path) {
		std::ifstream infile(path);
		return infile.good();
	}

	template <class P>
	auto switch_file(
		const P& filename, 
		const P& official_directory, 
		const P& custom_directory
	) {
		const auto custom = custom_directory + filename;

		if (file_exists(custom)) {
			return custom;
		}
		else {
			const auto official = official_directory + filename;
			ensure(file_exists(official));
			return official;
		}
	}

	template <class P>
	auto get_extension(const P& path) {
		return fs::path(path).extension().string();
	}

	template <class P, class S>
	auto replace_extension(const P& path, const S& new_ext) {
		return fs::path(path).replace_extension(new_ext).string();
	}

	template <class P, class S>
	auto replace_filename(const P& path, const S& new_fn) {
		return fs::path(path).replace_filename(new_fn).string();
	}

	template <class P>
	auto get_filename(const P& path) {
		return fs::path(path).filename().string();
	}

	template <class P>
	auto get_stem(const P& path) {
		return fs::path(path).stem().string();
	}

	template <class P>
	void ensure_existence(const P& path) {
		const bool exists = file_exists(path);

		if (!exists) {
			LOG("File not found: %x", path);
			ensure(exists);
		}
	}
	
	template <class P, class C = char>
	auto get_file_contents(const P& path, const C = C()) {
		std::basic_ifstream<C> t(path);
		std::basic_stringstream<C> buffer;
		buffer << t.rdbuf();

		return buffer.str();
	}

	template <class P>
	std::vector<std::string> get_file_lines_without_blanks_and_comments(
		const P& path,
		const char comment_begin_character = '%'
	) {
		std::ifstream input(filename);
	
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

	template <class P>
	auto get_file_lines(const P& path) {
		typedef std::string string_type;

		ensure_existence(path);
		std::ifstream input(path);

		std::vector<string_type> out;

		for (string_type line; std::getline(input, line);) {
			out.push_back(line);
		}

		return out;
	}

	template <class P, class S>
	void create_text_file(const P& path, const S& text) {
		std::ofstream out(path, std::ios::out);
		out << text;
	}

	template <class P, class S>
	void create_text_file_if_different(const P& path, const S& text) {
		if (!file_exists(path) || text != get_file_contents(path)) {
			std::ofstream out(path, std::ios::out);
			out << text;
		}
	}

	template <class P, class C>
	void create_binary_file(const P& path, const C& content) {
		std::ofstream out(path, std::ios::out | std::ios::binary);
		out.write(content.data(), content.size());
	}

	template <class P, class S>
	void get_file_contents_binary_into(const P& path, S& target) {
		ensure_existence(path);
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		const std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		target.reserve(static_cast<unsigned>(size));
		file.read(target.data(), size);
		target.set_write_pos(static_cast<size_t>(size));
	}

	template <class P, class ContainerType>
	void read_map_until_eof(const P& path, ContainerType& into) {
		std::ifstream source(path, std::ios::in | std::ios::binary);

		while (source.peek() != EOF) {
			typename ContainerType::key_type key;
			typename ContainerType::mapped_type value;

			augs::read(source, key);
			augs::read(source, value);

			into.emplace(std::move(key), std::move(value));
		}
	}
}