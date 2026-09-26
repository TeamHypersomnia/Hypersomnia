#pragma once
#include <algorithm>
#include <cstdint>
#include "augs/math/rects.h"
#include "augs/drawing/drawing.h"
#include "augs/texture_atlas/atlas_entry.h"

class cosmos;
class interpolation_system;
class visible_entities;

/*
	Environment shadows are cast by physical bodies that bullets can't fly over,
	under a single distant sun given by cosmos_light_settings::shadow_step.

	casts_output: every caster's fixture swept along its shadow,
	colored (shadow height, strength, 0, 0) with the strength fading towards the far end.
	Drawn with max blending, each pixel keeps the tallest height and the strongest shadow over it.

	footprints_output: every physical body's fixture as it is, colored (0, 0, shadow height, 0).
	Drawn with max blending too, it fills the blue channel without touching the casts.
	Visible NO_SHADOW areas go there too, as the highest footprint (0, 0, 255, 0) - nothing is taller, so the sun never reaches under them.
*/

/*
	Characters have no footprints in the shadow texture - their sprites stick out of their bodies -
	so they receive environment shadows at a fixed shadow height instead.
*/

inline constexpr float CHARACTER_SHADOW_HEIGHT = 2.0f;

/*
	Foreground sprites are drawn above characters, so their shadows always come from above them too.
*/

inline constexpr float FOREGROUND_SHADOW_BASE_HEIGHT = CHARACTER_SHADOW_HEIGHT + 1.0f;

inline uint8_t calc_foreground_shadow_height(const uint8_t extra_height) {
	return static_cast<uint8_t>(std::min(255.0f, FOREGROUND_SHADOW_BASE_HEIGHT + extra_height));
}

struct draw_environment_shadows_input {
	const cosmos& cosm;
	const interpolation_system& interp;
	const visible_entities& visible;
	const ltrb queried_camera_aabb;
	const augs::atlas_entry blank_tex;
	const float tip_strength;

	augs::vertex_triangle_buffer& casts_output;
	augs::vertex_triangle_buffer& footprints_output;
};

void draw_environment_shadows(const draw_environment_shadows_input in);
