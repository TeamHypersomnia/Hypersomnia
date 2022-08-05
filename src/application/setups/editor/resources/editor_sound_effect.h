#pragma once
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "game/detail/view_input/sound_effect_modifier.h"

struct editor_sound_effect : sound_effect_modifier {
	using base = sound_effect_modifier;
	using introspect_base = base;

	// GEN INTROSPECTOR struct editor_sound_effect
	editor_typed_resource_id<editor_sound_resource> resource_id;
	// END GEN INTROSPECTOR
};

