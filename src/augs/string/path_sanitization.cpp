#include "augs/string/path_sanitization.h"
#include "augs/network/network_types.h"

namespace sanitization {
	std::string describe(const forbidden_path_type f) {
		switch (f) {
			case forbidden_path_type::DOTS_OUTSIDE_EXTENSION:
				return "File path cannot have dots except one for the extension.";
			case forbidden_path_type::EMPTY:
				return "File path cannot be empty.";
			case forbidden_path_type::PART_EMPTY:
				return "File path cannot have zero-length elements.";
			case forbidden_path_type::FORBIDDEN_CHARACTERS:
				return "File and folder names can only contain lowercase alphanumeric characters and underscores (\"_\").";
			case forbidden_path_type::NO_EXTENSION:
				return "Filename must have an extension.";
			case forbidden_path_type::FORBIDDEN_EXTENSION:
				return "Filename must have an allowed extension.";
			case forbidden_path_type::TOO_LONG:
				return "File path cannnot be too long.";
			case forbidden_path_type::GOES_BEYOND_PARENT_FOLDER:
				return "File path goes beyond the parent folder.";
			default:
				return "Unknown forbidden path type.";
		}

	}

	namespace detail {
		bool only_alphanumeric_characters(const std::string& untrusted) {
			return std::string::npos == untrusted.find_first_not_of(portable_alphanumeric_set);
		}

		bool is_subpath_safe(
			const augs::path_type& parent_dir,
			const std::string& untrusted_subpath
		) {
			const std::filesystem::path parent_resolved = std::filesystem::canonical(parent_dir);
			const std::filesystem::path requested_file_path = std::filesystem::weakly_canonical(parent_resolved / untrusted_subpath);

			return std::equal(
				parent_resolved.begin(),
				parent_resolved.end(),
				requested_file_path.begin()
			);
		}

		bool all_path_elements_nonempty(const std::vector<std::string>& parts) {
			for (const std::string& part : parts) {
				if (part.empty()) {
					return false;
				}
			}

			return true;
		}

		bool all_path_elements_alphanumeric(const std::vector<std::string>& parts) {
			for (const std::string& part : parts) {
				if (!only_alphanumeric_characters(part)) {
					return false;
				}
			}

			return true;
		}

		int dots_to_forward_slashes(std::string& untrusted) {
			int dot_count = 0;

			for (char& c : untrusted) {
				if (c == '.') {
					c =  '/';

					++dot_count;
				}
			}

			return dot_count;
		}

		std::vector<std::string> split_by_forward_slashes(const std::string& untrusted) {
			std::vector<std::string> parts;
			const char delimiter = '/';

			std::istringstream split(untrusted);

			for (
				std::string each;
				std::getline(split, each, delimiter); 
				parts.push_back(each)
			);

			return parts;
		}

		result_or_error make_safe_file_path(std::string untrusted) {
			if (untrusted.size() > max_arena_file_path_length_v) {
				return forbidden_path_type::TOO_LONG;
			}

			if (untrusted.empty()) {
				return forbidden_path_type::EMPTY;
			}

			/* 
				For simplicity, we're delimiting all parts by the same character - forward slash.
				We'll just treat the last part as the file extension.
			*/

			{
				const int num_dots = dots_to_forward_slashes(untrusted);

				if (num_dots > 1) {
					return forbidden_path_type::DOTS_OUTSIDE_EXTENSION;
				}

				if (untrusted.front() == '/' || untrusted.back() == '/') {
					return forbidden_path_type::PART_EMPTY;
				}

				if (num_dots == 0) {
					return forbidden_path_type::NO_EXTENSION;
				}
			}


			auto parts = split_by_forward_slashes(untrusted);

			if (!all_path_elements_nonempty(parts)) {
				return forbidden_path_type::PART_EMPTY;
			}

			if (!all_path_elements_alphanumeric(parts)) {
				return forbidden_path_type::FORBIDDEN_CHARACTERS;
			}

			if (parts.size() < 2) {
				/* At least the filename and the extension. */
				return forbidden_path_type::NO_EXTENSION;
			}

			const std::string extension = parts.back();
			parts.pop_back();

			if (!is_whitelisted_extension(extension)) {
				return forbidden_path_type::FORBIDDEN_EXTENSION;
			}

			augs::path_type final_path = parts.front();

			for (std::size_t i = 1; i < parts.size(); ++i) {
				final_path /= parts[i];
			}

			final_path += '.';
			final_path += extension;

			return final_path;
		}
	}

	result_or_error sanitize_downloaded_file_path(
		const augs::path_type& project_dir,
		const std::string& untrusted_file_path
	) {
		const auto result = detail::make_safe_file_path(untrusted_file_path);

		return std::visit([&project_dir](auto sanitization_result) -> result_or_error {
			using T = decltype(sanitization_result);

			if constexpr(std::is_same_v<T, forbidden_path_type>) {
				return sanitization_result;
			}
			else {
				if (!detail::is_subpath_safe(project_dir, sanitization_result)) {
					return forbidden_path_type::GOES_BEYOND_PARENT_FOLDER;
				}

				return sanitization_result;
			}
		}, result);
	}

	result_or_error sanitize_map_name(
		const augs::path_type& maps_directory,
		const std::string& untrusted_map_name
	) {
		if (untrusted_map_name.size() == 0) {
			return forbidden_path_type::EMPTY;
		}

		if (untrusted_map_name.size() > max_arena_name_length_v) {
			return forbidden_path_type::TOO_LONG;
		}

		if (!detail::only_alphanumeric_characters(untrusted_map_name)) {
			return forbidden_path_type::FORBIDDEN_CHARACTERS;
		}

		if (!detail::is_subpath_safe(maps_directory, untrusted_map_name)) {
			return forbidden_path_type::GOES_BEYOND_PARENT_FOLDER;
		}

		return augs::path_type(untrusted_map_name);
	}
}

#if BUILD_UNIT_TESTS
#include "augs/log.h"
#include <Catch/single_include/catch2/catch.hpp>

TEST_CASE("Map sanitization test") {
	namespace S = sanitization;
	using F = S::forbidden_path_type;
	using R = S::result_or_error;

	std::string with_zero = "cyber";
	with_zero += '\0';
	with_zero += "aqua";

	const augs::path_type parent = DETAIL_DIR;

	REQUIRE(S::sanitize_map_name(parent, "") == R(F::EMPTY));
	REQUIRE(S::sanitize_map_name(parent, "cyberaqua") == R(augs::path_type("cyberaqua")));
	REQUIRE(S::sanitize_map_name(parent, "/cyberaqua") == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_map_name(parent, with_zero) == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_map_name(parent, "cyberaqua/") == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_map_name(parent, "/cyberaqua/cyberaqua/cyberaqua/cyberaqua/cyberaqua/cyberaqua") == R(F::TOO_LONG));
	REQUIRE(S::sanitize_map_name(parent, "kjlgfkjlkdfj893jdlksjdlfkj8934jfdskljfklgdfhlkj4jklfkjhyberaqua") == R(F::TOO_LONG));
	REQUIRE(S::sanitize_map_name(parent, "cyber aqua") == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_map_name(parent, "cyberakła") == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_map_name(parent, ".") == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_map_name(parent, "../") == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_map_name(parent, "....//") == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_map_name(parent, "..\\") == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_map_name(parent, ".\\") == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_map_name(parent, "..") == R(F::FORBIDDEN_CHARACTERS));
}

TEST_CASE("File path sanitization test") {
	namespace S = sanitization;
	using F = S::forbidden_path_type;
	using R = S::result_or_error;

	std::string with_zero = "cyber";
	with_zero += '\0';
	with_zero += "aqua.png";

	const augs::path_type parent = DETAIL_DIR;

	auto analyze = [&](auto r) {
		if (std::get_if<F>(&r)) {
			LOG_NVPS(describe(*std::get_if<F>(&r)));
		}
		else {
			LOG_NVPS(*std::get_if<augs::path_type>(&r));
		}
	};

	(void)analyze;

	analyze(S::sanitize_downloaded_file_path(parent, with_zero));

	REQUIRE(S::sanitize_downloaded_file_path(parent, "") == R(F::EMPTY));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "/cyberaqua.jpg") == R(F::PART_EMPTY));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "cyberaqua.png") == R(augs::path_type("cyberaqua.png")));
	REQUIRE(S::sanitize_downloaded_file_path(parent, with_zero) == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "cyberaqua/.jpg") == R(F::PART_EMPTY));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "//cyberaqua/cyberaqua/cyberaqua/cyberaqua/cyberaqua/cyberaquacyberaqua/cyberaqua/cyberaqua/cyberaqua/cyberaqua/cyberaquakfdlkjghhhhkfjhfklgjhlgfkhjl.wav") == R(F::TOO_LONG));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "kkjlgfkjlkdfj893jdlksjdlfkj8934jfdskljfklgdfhlkj4jklfkjhyberaquakjlgfkjlkdfj893jdlksjdlfkj8934jfdskljfklgdfhlkj4jklfkjhyberaquakjlgfkjlkdfj893jdlksjdlfkj8934jfdskljfklgdfhlkj4jklfkjhyberaquajlgfkjlkdfj893jdlksjdlfkj8934jfdskljfklgdfhlkj4jklfkjhyberaqua.wav") == R(F::TOO_LONG));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "") == R(F::EMPTY));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "cyber aqua.jpg") == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "cyberakła.png") == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_downloaded_file_path(parent, ".") == R(F::PART_EMPTY));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "../") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "....//") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "..\\") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, ".\\") == R(F::PART_EMPTY));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "..") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, ".") == R(F::PART_EMPTY));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "..\\") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "....\\\\\\\\") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "..\\\\") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, ".\\") == R(F::PART_EMPTY));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "..") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "\\") == R(F::NO_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "\\/\\") == R(F::NO_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "\\/\\.png") == R(F::FORBIDDEN_CHARACTERS));

	REQUIRE(S::sanitize_downloaded_file_path(parent, ".abc.png") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "../abc.png") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "....//abc.png") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "..\\abc.png") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, ".\\abc.png") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "..abc.png") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, ".abc.png") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "..\\abc.png") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "....\\\\\\\\abc.png") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "..\\\\abc.png") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, ".\\abc.png") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "..abc.png") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "\\abc.png") == R(F::FORBIDDEN_CHARACTERS));

	REQUIRE(S::sanitize_downloaded_file_path(parent, "/a/b/c/d.png") == R(F::PART_EMPTY));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a/b/c/d") == R(F::NO_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a/b/c/d.exe") == R(F::FORBIDDEN_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a/b/c/d.abc") == R(F::FORBIDDEN_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "abc.abc") == R(F::FORBIDDEN_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "abc.com") == R(F::FORBIDDEN_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "abc.lua") == R(augs::path_type("abc.lua")));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a/b/c/d/e/f/g/cos.ogg") == R(augs::path_type("a/b/c/d/e/f/g/cos.ogg")));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a\\b\\c\\d\\e\\f\\g\\cos.ogg") == R(F::FORBIDDEN_CHARACTERS));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a\\b\\c\\d\\e\\f\\g\\cos.test.ogg") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a\\b\\c\\d\\e\\f\\g\\") == R(F::NO_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a\\b\\c\\d\\e\\f\\g") == R(F::NO_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a/b/c/d/e/f/g/cos.ogg.ogg") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a/b/c/d/e/f/g/cos..ogg") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a/b/c/d/e/f/g/..ogg") == R(F::DOTS_OUTSIDE_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a/b/c/d/e/f/g/.ogg") == R(F::PART_EMPTY));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a/b/c/d/e/f/g/ogg.") == R(F::PART_EMPTY));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a/b/c/d/e/f/g/ogg/") == R(F::PART_EMPTY));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a/b/c/d/e/f/g/ogg") == R(F::NO_EXTENSION));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a/b/c/d/e/f/g/abcabc.") == R(F::PART_EMPTY));
	REQUIRE(S::sanitize_downloaded_file_path(parent, "a/b/c/d/e/f/g/exe.") == R(F::PART_EMPTY));

}

#endif
