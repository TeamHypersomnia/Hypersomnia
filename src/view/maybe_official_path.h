#pragma once
#include "augs/filesystem/path.h"
#include "view/maybe_official_path_declaration.h"

std::string format_field_name(std::string s);

template <class T>
struct maybe_official_path {
	using id_type = T;

	// GEN INTROSPECTOR struct maybe_official_path class T
	augs::path_type path;
	bool is_official = false;
	// END GEN INTROSPECTOR

	bool operator==(const maybe_official_path& b) const {
		return path == b.path && is_official == b.is_official;
	}

	bool operator!=(const maybe_official_path& b) const {
		return !operator==(b);
	}

	bool operator<(const maybe_official_path& b) const {
		if (is_official == b.is_official) {
			return path < b.path;
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

	template <class E>
	static auto is_supported_extension(const E& ext) {
		if constexpr(std::is_same_v<T, assets::image_id>) {
			return ext == ".png";
		}
		else if constexpr(std::is_same_v<T, assets::sound_id>) {
			return ext == ".wav" || ext == ".ogg";
		}
		else {
			static_assert(always_false_v<T>, "Unsupported id type.");
		}
	}

	static auto get_label() {
		if constexpr(std::is_same_v<T, assets::image_id>) {
			return "image";
		}
		else if constexpr(std::is_same_v<T, assets::sound_id>) {
			return "sound";
		}
		else {
			static_assert(always_false_v<T>, "Unsupported id type.");
		}
	}

	static auto get_content_suffix() {
		if constexpr(std::is_same_v<T, assets::image_id>) {
			return "gfx";
		}
		else if constexpr(std::is_same_v<T, assets::sound_id>) {
			return "sfx";
		}
		else {
			static_assert(always_false_v<T>, "Unsupported id type.");
		}
	}

	static auto get_in_official() {
		return augs::path_type(OFFICIAL_CONTENT_DIR) / get_content_suffix();
	}

	auto resolve(const augs::path_type& project_dir) const {
		if (is_official || project_dir.empty()) {
			return get_in_official() / path;
		}

		return project_dir / get_content_suffix() / path;
	}
};

template <class T>
struct is_maybe_official_path : std::false_type {};

template <class T>
struct is_maybe_official_path<maybe_official_path<T>> : std::true_type {};
