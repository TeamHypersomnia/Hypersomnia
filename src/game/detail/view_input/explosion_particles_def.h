#pragma once
#include <cstddef>
#include <algorithm>
#include "augs/pad_bytes.h"
#include "augs/math/arithmetical.h"
#include "augs/graphics/rgba.h"
#include "augs/misc/constant_size_vector.h"

constexpr std::size_t MAX_EXPLOSION_PARTICLES_PALETTE_COLORS = 6;
using explosion_particles_palette = augs::constant_size_vector<rgba, MAX_EXPLOSION_PARTICLES_PALETTE_COLORS>;

/*
	The color at a fractional position in the palette (in entries), clamped to its ends. Always opaque.
*/

inline rgba sample_explosion_particles_palette(const explosion_particles_palette& palette, const float pos) {
	if (palette.empty()) {
		return white;
	}

	const auto last = static_cast<float>(palette.size() - 1);
	const auto clamped = std::clamp(pos, 0.f, last);
	const auto i = static_cast<std::size_t>(clamped);
	const auto next = std::min(i + 1, static_cast<std::size_t>(palette.size() - 1));

	auto result = augs::interp(palette[i], palette[next], clamped - static_cast<float>(i));
	result.a = 255;

	return result;
}

/*
	How an explosion's pixel art particles look and move, on top of the client's explosions_settings
	(see exploding_ring_system).

	enabled spawns the particles at all - without them, the explosion still has its thin rings and the flash.
	thin_ring_thickness_mult scales the client's thickness of the explosion's thin rings.

	variation scales their randomness (jitter, random acceleration, lifetime spread) -
	lower for the orderly, cybernetic-looking blasts.
	lifetime_mult scales how long they live,
	ease_out_mult the time constant of their slowdown - more is gentler,
	so they keep moving past the ring's end instead of stalling there.

	They start in the explosion's inner ring color, slightly brightened, then take their ring's color.
	cooling scales how much they darken as they die out (0 = not at all),
	cool_color, if set (non-zero alpha), is the color they end in instead of darkening,
	cool_from_mult scales the moment they start cooling down (less = sooner).

	A non-empty palette replaces all the above coloring:
	the particles go through its colors smoothly over their lifetime (the outer ring half a step ahead),
	and the explosion's flash and thin rings take its first color.

	fire picks the client's fire particle settings (size, speeds, thin rings etc.) instead of the standard ones,
	regardless of the colors.

	playback_speed plays the whole effect - the rings and their particles - that many times faster,
	keeping its structure: the particles trace exactly the same paths, in the same colors, only sooner.
*/

struct explosion_particles_def {
	// GEN INTROSPECTOR struct explosion_particles_def
	real32 variation = 1.f;
	real32 thin_ring_thickness_mult = 1.f;
	real32 lifetime_mult = 1.f;
	real32 ease_out_mult = 1.f;
	real32 cooling = 1.f;
	real32 cool_from_mult = 1.f;
	real32 playback_speed = 1.f;
	rgba cool_color = rgba(0, 0, 0, 0);
	explosion_particles_palette palette;
	bool fire = false;
	bool enabled = true;
	pad_bytes<2> pad;
	// END GEN INTROSPECTOR
};
