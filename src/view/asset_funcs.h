#pragma once
#include "game/assets/ids/asset_ids.h"

template <class T>
std::string get_content_suffix();

namespace assets {
	template <class T>
	std::string get_label();

	template <class T>
	bool is_supported_extension(const std::string& ext);
}
