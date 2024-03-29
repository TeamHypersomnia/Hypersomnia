#pragma once
#include "game/assets/ids/asset_ids.h"

namespace assets {
	template <class T>
	std::string get_label();

	template <class T>
	bool is_supported_extension(const std::string& ext);
	bool is_asset_extension(const std::string& ext);

	bool is_image_extension(const std::string& ext);
	bool is_sound_extension(const std::string& ext);
}
