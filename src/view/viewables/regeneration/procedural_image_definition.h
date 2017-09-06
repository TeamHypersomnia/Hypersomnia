#pragma once
#include <optional>

#include "view/viewables/regeneration/buttons_with_corners.h"
#include "view/viewables/regeneration/images_from_commands.h"

struct procedural_image_definition {
	// GEN INTROSPECTOR struct procedural_image_definition
	std::optional<button_with_corners_input> button_with_corners;
	std::optional<image_from_commands_input> image_from_commands;
	// END GEN INTROSPECTOR
};