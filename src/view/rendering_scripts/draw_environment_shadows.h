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
	The shadow texture reaches this many screen pixels past the two edges of the screen away from the sun.
	A receiver raised above the ground samples the shadows its height times the sun's step further from the sun -
	near those edges, beyond the screen - so it samples the shadows really lying there,
	not the screen's edge repeated. Farther samples are still clamped to the texture's edge.
*/

inline constexpr int SUN_SHADOW_GUARD_BAND_PX = 384;

struct sun_shadow_texture_layout {
	/* The world area the shadow texture covers. */
	ltrb world_rect;

	/* Added to the fragment coordinates of the screen to find their texel in the shadow texture. */
	vec2 texel_offset;
};

inline sun_shadow_texture_layout calc_sun_shadow_texture_layout(const ltrb visible_world_rect, const float zoom, const vec2 sun_step) {
	const auto band_px = static_cast<float>(SUN_SHADOW_GUARD_BAND_PX);
	const auto band_world = band_px / zoom;

	auto result = sun_shadow_texture_layout { visible_world_rect, vec2::zero };

	/*
		Fragment coordinates grow rightwards and upwards, while the world's y grows downwards.
	*/

	if (sun_step.x >= 0.0f) {
		result.world_rect.r += band_world;
	}
	else {
		result.world_rect.l -= band_world;
		result.texel_offset.x = band_px;
	}

	if (sun_step.y >= 0.0f) {
		result.world_rect.b += band_world;
		result.texel_offset.y = band_px;
	}
	else {
		result.world_rect.t -= band_world;
	}

	return result;
}

/*
	Environment shadows are cast by physical bodies that bullets can't fly over,
	under a single distant sun given by sun_shadow_settings::step.

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
	Shadows of foreground sprites fall on everything below them regardless of their height,
	which only decides how far they are thrown.
*/

inline constexpr float FOREGROUND_SHADOW_BASE_HEIGHT = 3.0f;

inline uint8_t calc_foreground_shadow_height(const uint8_t extra_height) {
	return static_cast<uint8_t>(std::min(255.0f, FOREGROUND_SHADOW_BASE_HEIGHT + extra_height));
}

/*
	Silhouette shadows share the alpha channel of the shadow texture, 7 bits of opacity each.
	Foreground ones occupy the upper half - they fall on everything below the foreground,
	whereas the ones of ground sprites fall only on the ground.
	Under max blending, foreground shadows win where the two overlap.
*/

inline uint8_t encode_silhouette_shadow_opacity(const uint8_t opacity, const bool is_foreground) {
	const auto level = static_cast<uint8_t>((opacity * 127 + 127) / 255);
	return is_foreground ? static_cast<uint8_t>(128 + level) : level;
}

struct draw_environment_shadows_input {
	const cosmos& cosm;
	const interpolation_system& interp;
	const visible_entities& visible;
	/* The world area of the shadow texture - see calc_sun_shadow_texture_layout. */
	const ltrb covered_world_rect;
	const augs::atlas_entry blank_tex;
	const float tip_strength;

	augs::vertex_triangle_buffer& casts_output;
	augs::vertex_triangle_buffer& footprints_output;
};

void draw_environment_shadows(const draw_environment_shadows_input in);
