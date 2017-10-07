#pragma once
#include "augs/graphics/rgba.h"
#include "augs/filesystem/path.h"
#include "augs/image/image.h"

struct image_from_commands_input {
	// GEN INTROSPECTOR struct image_from_commands_input
	std::vector<augs::paint_command_variant> commands;
	// END GEN INTROSPECTOR
};

using image_from_commands_stamp = image_from_commands_input;

void regenerate_image_from_commands(
	const augs::path_type& output_image_path,
	const image_from_commands_input& input,
	const bool force_regenerate
);