#include "augs/templates/identity_templates.h"
#include "view/asset_funcs.h"

template <class T>
std::string get_content_suffix() {
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

namespace assets {
	template <class T>
	std::string get_label() {
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

	template <class T>
	bool is_supported_extension(const std::string& ext) {
		if constexpr(std::is_same_v<T, assets::image_id>) {
			return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp";
		}
		else if constexpr(std::is_same_v<T, assets::sound_id>) {
			return ext == ".wav" || ext == ".ogg";
		}
		else {
			static_assert(always_false_v<T>, "Unsupported id type.");
		}
	}

	bool is_asset_extension(const std::string& ext) {
		return is_supported_extension<assets::image_id>(ext) || is_supported_extension<assets::sound_id>(ext);
	}
}

template std::string get_content_suffix<assets::image_id>();
template std::string get_content_suffix<assets::sound_id>();

template std::string assets::get_label<assets::image_id>();
template std::string assets::get_label<assets::sound_id>();

template bool assets::is_supported_extension<assets::image_id>(const std::string&);
template bool assets::is_supported_extension<assets::sound_id>(const std::string&);
