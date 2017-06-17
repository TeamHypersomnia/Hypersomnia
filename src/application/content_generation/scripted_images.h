#pragma once
#include "augs/graphics/rgba.h"
#include "augs/misc/templated_readwrite.h"
#include "augs/padding_byte.h"
#include "augs/image/image.h"

struct scripted_image_stamp {
	// GEN INTROSPECTOR struct scripted_image_stamp
	std::vector<augs::image::command_variant> commands;
	// END GEN INTROSPECTOR
};

void regenerate_scripted_images(
	const bool force_regenerate
);