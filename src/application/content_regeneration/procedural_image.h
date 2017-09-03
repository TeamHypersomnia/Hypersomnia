#pragma once
#include <optional>

#include "application/content_regeneration/buttons_with_corners.h"
#include "application/content_regeneration/scripted_images.h"

struct procedural_image_definition {
	// GEN INTROSPECTOR struct procedural_image_definition
	std::optional<button_with_corners_input> button_with_corners;
	std::optional<scripted_image_input> scripted_image;
	// END GEN INTROSPECTOR
};