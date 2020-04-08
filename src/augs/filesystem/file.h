#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>

#include <fstream>
#include <filesystem>

#include "augs/filesystem/path.h"

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

	using file_open_error = std::ifstream::failure;
	using filesystem_error = std::filesystem::filesystem_error;
	
	inline auto last_write_time(const path_type& path) {
		return std::filesystem::last_write_time(path);
	}

	inline bool exists(const path_type& path) {
		return std::filesystem::exists(path);
	}

	inline bool is_empty(const path_type& path) {
		return std::filesystem::is_empty(path);
	}

	inline decltype(auto) remove_file(const path_type& path) {
		std::error_code err;
		return std::filesystem::remove(path, err);
	}

	inline decltype(auto) remove_directory(const path_type& path) {
		std::error_code err;
		return std::filesystem::remove_all(path, err);
	}

	enum class free_path_type {
		NON_EXISTING,
		EMPTY
	};

	inline path_type switch_path(
		const path_type canon_path,
		const path_type local_path
	) {
		if (augs::exists(local_path)) {
			return local_path;
		}
		else {
			return canon_path;
		}
	}

	inline auto file_to_string(const path_type& path) {
		auto t = with_exceptions<std::ifstream>();
		t.open(path);

		std::stringstream buffer;
		buffer << t.rdbuf();

		return buffer.str();
	}

	inline auto file_to_lines(const path_type& path) {
		auto t = with_exceptions<std::ifstream>();
		t.open(path);

		auto out = std::vector<std::string>();

		for (auto line = std::string(); std::getline(t, line); ) {
			out.emplace_back(line);
		}

		return out;
	}

	inline bool file_exists_and_non_empty(const path_type& path) {
		try {
			return file_to_string(path).size() > 0;
		}
		catch(...) {
			return false;
		}
	}

	template <class S>
	void save_as_text(const path_type& path, const S& text) {
		auto out = with_exceptions<std::ofstream>();
		out.open(path, std::ios::out);
		out << text;
	}

	template <class S>
	void save_as_text_if_different(const path_type& path, const S& text) {
		if (!augs::exists(path) || text != file_to_string(path)) {
			auto out = with_exceptions<std::ofstream>();
			out.open(path, std::ios::out);
			out << text;
		}
	}

	inline auto get_file_size(const path_type& path) {
		auto in = with_exceptions<std::ifstream>(path, std::ifstream::ate | std::ifstream::binary);
		return in.tellg(); 
	};

}