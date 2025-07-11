#pragma once
#include <cstdint>
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "augs/audio/distance_model.h"
#include "augs/math/declare_math.h"
#include "augs/templates/maybe.h"

struct editor_sound_resource;

struct editor_sound_property_effect_modifier {
	// GEN INTROSPECTOR struct editor_sound_property_effect_modifier
	real32 gain = 1.0f;
	real32 pitch = 1.0f;
	// END GEN INTROSPECTOR

	bool operator==(const editor_sound_property_effect_modifier&) const = default;
};

struct editor_sound_effect_modifier {
	// GEN INTROSPECTOR struct editor_sound_effect_modifier
	real32 gain = 1.0f;
	real32 pitch = 1.0f;
	bool spatialize = true;
	augs::maybe<augs::distance_model> distance_model = { augs::distance_model::INVERSE_DISTANCE_CLAMPED, false };
	augs::maybe<real32> max_distance = { 4000.f, false };
	augs::maybe<real32> reference_distance = { 1000.f, false };
	real32 doppler_intensity = 1.0f;

	bool loop = false;
	uint32_t play_times = 1;
	// END GEN INTROSPECTOR

	bool operator==(const editor_sound_effect_modifier&) const = default;
};

struct editor_sound_effect : editor_sound_property_effect_modifier {
	using base = editor_sound_property_effect_modifier;
	using introspect_base = base;

	// GEN INTROSPECTOR struct editor_sound_effect
	editor_typed_resource_id<editor_sound_resource> id;
	// END GEN INTROSPECTOR

	bool is_set() const {
		return id.is_set();
	}

	bool operator==(const editor_sound_effect&) const = default;
};

struct editor_theme {
	// GEN INTROSPECTOR struct editor_theme
	editor_typed_resource_id<editor_sound_resource> id;
	real32 gain = 1.0f;
	real32 pitch = 1.0f;
	// END GEN INTROSPECTOR

	bool operator==(const editor_theme&) const = default;
};
