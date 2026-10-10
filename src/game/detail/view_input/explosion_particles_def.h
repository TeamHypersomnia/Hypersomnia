#pragma once
#include <cstddef>
#include <algorithm>
#include "augs/pad_bytes.h"
#include "augs/math/arithmetical.h"
#include "augs/graphics/rgba.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/bound.h"

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

/*
	A jet of the explosion particles shooting out of the muzzle, like the smokes of a muzzle flash.

	amount particles spawn anywhere in the ring between spawn_radius.first and spawn_radius.second
	around the point muzzle_offset px ahead of the muzzle, along the jet,
	then fly straight at speed (px/s) - in the muzzle's direction tilted by angular_offset
	and spread by at most spread degrees either way, both picked once per shot.
	Each lives lifetime_ms, going through the explosion particles' colors and shrinking over the longest lifetime.

	The particles are drawn along their flight, turned by at most rotation_variation degrees either way,
	their sprites scaled by size_mult - the explosion particles' sprites are made for big blasts.
	damping (per second) slows them down exponentially.
*/

struct particle_jet_def {
	// GEN INTROSPECTOR struct particle_jet_def
	augs::bound<real32> amount = { 0.f, 0.f };
	augs::bound<real32> spawn_radius = { 0.f, 0.f };
	augs::bound<real32> speed = { 0.f, 0.f };
	augs::bound<real32> angular_offset = { 0.f, 0.f };
	augs::bound<real32> spread = { 0.f, 0.f };
	augs::bound<real32> lifetime_ms = { 0.f, 0.f };
	real32 size_mult = 0.25f;
	real32 rotation_variation = 25.f;
	real32 damping = 0.f;
	real32 muzzle_offset = 5.f;
	// END GEN INTROSPECTOR

	real32 calc_max_reach() const {
		return muzzle_offset + spawn_radius.second + speed.second * lifetime_ms.second / 1000.f;
	}
};

constexpr std::size_t MAX_PARTICLE_EXPLOSION_JETS = 2;
using particle_jets_vector = augs::constant_size_vector<particle_jet_def, MAX_PARTICLE_EXPLOSION_JETS>;

/*
	A purely visual explosion of the pixel art particles, e.g. leaving the muzzle with the round.
	Its rings spawn only with radius above 0, ring_duration_seconds further shortened by particles.playback_speed.

	Any jets shoot out besides the rings - their particles go from inner_ring_color to outer_ring_color.

	A gun's own muzzle_leave_explosion, if set, replaces the one of its round,
	e.g. for guns sharing a round, but not its muzzle flash.
*/

struct particle_explosion_def {
	// GEN INTROSPECTOR struct particle_explosion_def
	real32 radius = 0.f;
	real32 ring_duration_seconds = 0.2f;
	rgba inner_ring_color = white;
	rgba outer_ring_color = white;
	explosion_particles_def particles;
	particle_jets_vector jets;
	// END GEN INTROSPECTOR

	bool is_set() const {
		return radius > 0.f || !jets.empty();
	}
};
