#pragma once
#include <experimental/filesystem>
#include "augs/readwrite/byte_readwrite_declaration.h"

namespace augs {
	using path_type = std::experimental::filesystem::path;

#if READWRITE_OVERLOAD_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

	template <class Archive>
	void read_object(
		Archive& ar,
		path_type& storage
	) {
		std::string str;
		read(ar, str);
		storage = str;
	}

	template <class Archive>
	void write_object(
		Archive& ar,
		const path_type& storage
	) {
		auto nice_representation = storage.string();
		
		for (auto& s : nice_representation) {
			/* Double backslash is ugly */
			
			if (s == '\\') {
				s = '/';
			}
		}

		write(ar, nice_representation);
	}

	template <class Archive>
	void write_object(Archive& ar, const std::string& storage) = delete;

	template <class Archive>
	void read_object(Archive& ar, std::string& storage) = delete;

	template <class Archive>
	void write_object(Archive& ar, const std::wstring& storage) = delete;

	template <class Archive>
	void read_object(Archive& ar, std::wstring& storage) = delete;

	inline auto to_display_path(path_type target_path) {
		auto display_path = target_path.filename();
		display_path += " (";
		display_path += target_path.replace_filename("");
		display_path += ")";
		return display_path;
	}
}

namespace std {
	template <>
	struct hash<augs::path_type> {
		size_t operator()(const augs::path_type& k) const {
			return std::experimental::filesystem::hash_value(k);
		}
	};
}