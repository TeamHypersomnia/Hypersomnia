#pragma once
#include <string>
#include <variant>
#include <vector>

#include "augs/filesystem/path.h"

namespace sanitization {
	enum class forbidden_path_type {
		DOTS_OUTSIDE_EXTENSION,
		EMPTY,
		PART_EMPTY,
		FORBIDDEN_CHARACTERS,
		NO_EXTENSION,
		FORBIDDEN_EXTENSION,
		TOO_LONG,
		GOES_BEYOND_PARENT_FOLDER,
		TOTAL_PATH_IS_TOO_LONG
	};

	std::string describe(forbidden_path_type);

	using result_or_error = std::variant<forbidden_path_type, augs::path_type>;

	namespace detail {
		constexpr auto portable_alphanumeric_set = "_abcdefghijklmnopqrstuvwxyz0123456789";

		inline bool is_whitelisted_extension(const std::string& ext) {
			return
				ext == "png"  ||
				ext == "gif"  ||
				ext == "jpg"  ||
				ext == "jpeg" ||
				ext == "wav"  ||
				ext == "ogg"  ||
				ext == "lua"
			;
		}

		bool is_whitelisted_extension(const std::string& ext);
		bool only_alphanumeric_characters(const std::string& untrusted);

		bool is_subpath_within_parent(
			const augs::path_type& parent_dir,
			const augs::path_type& untrusted_subpath
		);

		bool is_absolute_valid_length(
			const augs::path_type& parent_dir,
			const augs::path_type& untrusted_subpath
		);

		bool all_path_elements_nonempty(const std::vector<std::string>& parts);
		bool all_path_elements_alphanumeric(const std::vector<std::string>& parts);
		int dots_to_forward_slashes(std::string& untrusted);
		std::vector<std::string> split_by_forward_slashes(const std::string& untrusted);
		result_or_error make_safe_file_path(std::string untrusted);
	}

	result_or_error sanitize_downloaded_file_path(
		const augs::path_type& project_dir,
		const std::string& untrusted_file_path
	);

	result_or_error sanitize_map_name(
		const augs::path_type& maps_directory,
		const std::string& untrusted_map_name
	);
}

