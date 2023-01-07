#include "augs/templates/identity_templates.h"
#include "view/asset_funcs.h"

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
			return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp" || ext == ".gif";
		}
		else if constexpr(std::is_same_v<T, assets::sound_id>) {
			return ext == ".wav" || ext == ".ogg";
		}
		else {
			static_assert(always_false_v<T>, "Unsupported id type.");
		}
	}

	bool is_image_extension(const std::string& ext) {
		return is_supported_extension<assets::image_id>(ext);
	}

	bool is_sound_extension(const std::string& ext) {
		return is_supported_extension<assets::sound_id>(ext);
	}

	bool is_asset_extension(const std::string& ext) {
		return is_image_extension(ext) || is_sound_extension(ext);
	}
}

template std::string assets::get_label<assets::image_id>();
template std::string assets::get_label<assets::sound_id>();

template bool assets::is_supported_extension<assets::image_id>(const std::string&);
template bool assets::is_supported_extension<assets::sound_id>(const std::string&);
