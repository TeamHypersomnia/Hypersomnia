#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <chrono>
#include <algorithm>

#include <fstream>
#include <experimental/filesystem>

#include "augs/string/typesafe_sprintf.h"
#include "augs/filesystem/path.h"
#include "augs/templates/identity_templates.h"

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
	using filesystem_error = std::experimental::filesystem::filesystem_error;
	
	inline auto last_write_time(const path_type& path) {
		return std::experimental::filesystem::last_write_time(path);
	}

	inline bool exists(const path_type& path) {
		return std::experimental::filesystem::exists(path);
	}

	inline bool is_empty(const path_type& path) {
		return std::experimental::filesystem::is_empty(path);
	}

	inline decltype(auto) remove_file(const path_type& path) {
		std::error_code err;
		return std::experimental::filesystem::remove(path, err);
	}

	inline decltype(auto) remove_directory(const path_type& path) {
		std::error_code err;
		return std::experimental::filesystem::remove_all(path, err);
	}

	enum class free_path_type {
		NON_EXISTING,
		EMPTY
	};

	template <class F = true_returner>
	path_type find_first(const free_path_type type, const path_type& path_template, F&& allow_candidate = true_returner()) {
		for (std::size_t candidate = 0;; ++candidate) {
			const auto candidate_path = candidate ? 
				typesafe_sprintf(path_template.string(), typesafe_sprintf("-%x", candidate))
				: typesafe_sprintf(path_template.string(), "")
			;

			auto result = [&]() {
				if (!augs::exists(candidate_path)) {
					return candidate_path;
				}

				if (type == free_path_type::EMPTY) {
					if (augs::is_empty(candidate_path)) {
						return candidate_path;
					}
				}

				return std::string();
			}();

			if (result.empty()) {
				continue;
			}

			if (allow_candidate(result)) {
				return result;
			}
		}
	}

	template <class... Args>
	decltype(auto) first_empty_path(Args&&... args) {
		return find_first(free_path_type::EMPTY, std::forward<Args>(args)...);
	}

	template <class... Args>
	decltype(auto) first_free_path(Args&&... args) {
		return find_first(free_path_type::NON_EXISTING, std::forward<Args>(args)...);
	}

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
}