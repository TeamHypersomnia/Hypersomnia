#pragma once
#include <cstdint>
#include "augs/filesystem/path.h"
#include "all_paths.h"

std::string format_field_name(std::string s);

namespace augs {
	bool natural_order(const std::string& a, const std::string& b);
}

template <class T>
struct maybe_official_path {
	using id_type = T;

	enum : uint8_t {
		CUSTOM = 0,
		OFFICIAL = 1,
		RESOLVED = 2
	};

	// GEN INTROSPECTOR struct maybe_official_path class T
	augs::path_type path;
	uint8_t is_official = 0;
	// END GEN INTROSPECTOR

	bool operator==(const maybe_official_path& b) const {
		return path == b.path && is_official == b.is_official;
	}

	bool operator!=(const maybe_official_path& b) const {
		return !operator==(b);
	}

	bool operator<(const maybe_official_path& b) const {
		if (is_official == b.is_official) {
			return augs::natural_order(path.string(), b.path.string());
		}

		return (is_official ? 1 : 0) < (b.is_official ? 1 : 0);
	}

	auto suffixed(std::string p) const {
		if (p.size() > 0) {
			p += " ";
		}

		return p + (is_official ? "(Official)" : "(Project)");
	}

	auto filename_first() const {
		return suffixed(augs::filename_first(path));
	}

	auto get_prettified_full() const {
		return suffixed(augs::get_prettified_full(path));
	}

	auto get_prettified_filename() const {
		return augs::get_prettified_filename(path);
	}

	auto resolve(const augs::path_type& project_dir) const {
		if (is_official == RESOLVED) {
			return path;
		}

		if (is_official || project_dir.empty()) {
			return OFFICIAL_CONTENT_DIR / path;
		}

		return project_dir / path;
	}
};

template<class T>
std::ostream& operator<<(std::ostream& out, const maybe_official_path<T>& x) {
	return out << typesafe_sprintf("%x (%x)", x.path, x.is_official ? "official" : "project");
}

template <class T>
struct is_maybe_official_path : std::false_type {};

template <class T>
struct is_maybe_official_path<maybe_official_path<T>> : std::true_type {};
