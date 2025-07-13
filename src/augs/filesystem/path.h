#pragma once
#include <cstddef>
#include "augs/filesystem/path_declaration.h"
#include "augs/readwrite/byte_readwrite_declaration.h"
#include "augs/string/string_templates.h"

std::string to_forward_slashes(std::string);
std::string format_field_name(std::string s);

#if PLATFORM_WINDOWS
namespace augs {
	std::string wstr_to_utf8(const wchar_t *wstr);
}

#endif

namespace augs {
#if READWRITE_OVERLOAD_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

#if PLATFORM_WINDOWS
	inline augs::path_type path_windows_friendly(const std::string& s_utf8) {
		return std::filesystem::u8path(s_utf8);
	}

	inline std::string string_windows_friendly(const augs::path_type& p_wide) {
		return wstr_to_utf8(p_wide.c_str());
	}

	inline augs::path_type make_windows_friendly(const augs::path_type& p_wide) {
		return std::filesystem::u8path(string_windows_friendly(p_wide));
	}
#else
	inline augs::path_type path_windows_friendly(const std::string& s_utf8) {
		return s_utf8;
	}

	inline std::string string_windows_friendly(const augs::path_type& p_wide) {
		return p_wide.string();
	}

	inline augs::path_type make_windows_friendly(const augs::path_type& p_wide) {
		return p_wide;
	}
#endif

	template <class Archive>
	void read_object_bytes(
		Archive& ar,
		path_type& storage
	) {
		std::string str;
		augs::read_bytes(ar, str);
		storage = str;
	}

	template <class Archive>
	void write_object_bytes(
		Archive& ar,
		const path_type& storage
	) {
		augs::write_bytes(ar, to_forward_slashes(storage.string()));
	}

	template <class Archive>
	void write_object_bytes(Archive& ar, const std::string& storage) = delete;

	template <class Archive>
	void read_object_bytes(Archive& ar, std::string& storage) = delete;

	/*
		"/a/b/c/cyan_charge.png" -> " (/a/b/c)"
	*/
	inline std::string parenthesized_dir(path_type target_path) {
		if (const auto directory = target_path.replace_filename("").string();
			!directory.empty()
		) {
			return " (" + directory + ")";
		}

		return "";
	}

	/*
		"/a/b/c/cyan_charge.png" -> "Cyan charge"
	*/
	inline auto get_prettified_filename(const path_type& target_path) {
		return format_field_name(target_path.stem().string());
	}

	/*
		"/a/b/c/cyan_charge.png" -> "Cyan charge (/a/b/c)"
	*/

	inline auto get_prettified_full(const path_type& target_path) {
		return get_prettified_filename(target_path) + parenthesized_dir(target_path);
	}

	/*
		"/a/b/c/cyan_charge.png" -> "cyan_charge.png (/a/b/c)"
	*/
	inline auto filename_first(const path_type& target_path) {
		return target_path.filename().string() + parenthesized_dir(target_path);
	}

}

#if !PLATFORM_WEB && !PLATFORM_WINDOWS
#if defined(__clang__)
#if __clang_major__ < 17
namespace std {
	template <>
	struct hash<augs::path_type> {
		size_t operator()(const augs::path_type& k) const {
			return std::filesystem::hash_value(k);
		}
	};
}
#endif
#endif
#endif
