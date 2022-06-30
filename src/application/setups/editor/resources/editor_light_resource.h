#pragma once
#include "augs/math/vec2.h"
#include "game/components/light_component.h"

using editor_light_resource_editable = components::light;

struct editor_light_resource {
	editor_light_resource_editable editable;

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}
};
