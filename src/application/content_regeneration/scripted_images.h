#pragma once
#include "augs/graphics/rgba.h"
#include "augs/misc/templated_readwrite.h"
#include "augs/pad_bytes.h"
#include "augs/image/image.h"

struct scripted_image_input {
	// GEN INTROSPECTOR struct scripted_image_input
	std::vector<augs::paint_command_variant> commands;
	// END GEN INTROSPECTOR
};

using scripted_image_stamp = scripted_image_input;

void regenerate_scripted_image(
	const std::string& output_image_path,
	const scripted_image_input& input,
	const bool force_regenerate
);