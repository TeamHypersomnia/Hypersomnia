#pragma once
#include "application/setups/editor/resources/editor_pathed_resource.h"

#include "game/detail/view_input/sound_effect_modifier.h"

struct editor_sound_resource {
	// GEN INTROSPECTOR struct editor_sound_resource
	editor_pathed_resource external_file;

	sound_effect_modifier modifier;
	// END GEN INTROSPECTOR

	editor_sound_resource(const editor_pathed_resource& f) : external_file(f) {}
};
