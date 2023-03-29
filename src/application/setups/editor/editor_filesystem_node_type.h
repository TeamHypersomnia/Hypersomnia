#pragma once
#include "view/asset_funcs.h"

enum class editor_filesystem_node_type : uint8_t {
	OTHER_FILE,
	FOLDER,

	IMAGE,
	SOUND,

	OTHER_RESOURCE
};

inline editor_filesystem_node_type get_filesystem_node_type_by_extension(const std::string& extension) {
	if (assets::is_image_extension(extension) || extension == ".gif") {
		return editor_filesystem_node_type::IMAGE;
	}

	if (assets::is_sound_extension(extension)) {
		return editor_filesystem_node_type::SOUND;
	}

	return editor_filesystem_node_type::OTHER_FILE;
}
