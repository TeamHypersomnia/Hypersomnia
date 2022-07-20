#pragma once
#include "augs/templates/enum_introspect.h"
#include "game/assets/ids/asset_ids.h"
#include "augs/misc/maybe_official_path.h"

template <class T>
auto enum_to_image_stem(const T& enum_id) {
	return to_lowercase(augs::enum_to_string(enum_id));
}

inline auto enum_string_to_image_path(const std::string& stringized_enum) {
	return maybe_official_path<assets::image_id>{ augs::path_type("gfx") / (to_lowercase(stringized_enum) + ".png"), true };
}

template <class T>
auto enum_to_image_path(const T& enum_id) {
	return maybe_official_path<assets::image_id>{ augs::path_type("gfx") / (enum_to_image_stem(enum_id) + ".png"), true };
}
