#pragma once
#include <optional>
#include <vector>
#include "test_scenes/test_scene_flavour_ids.h"
#include "test_scenes/test_scene_particle_effects.h"

/*
	Shorter and thinner bullet line trails, with the neon glows of their rounds shortened to match.
	The long ones obscured too much.

	Trails are scaled in load_test_scene_particle_effects, neons in the guns' ingredients,
	both in final passes after all the effects and rounds were cloned from each other.

	trace - the trace effect containing the line trail.
	rounds - the rounds whose neon glow extends backward over this trail.
	length_mult - lifetime of the trail segments.
	thickness_mult - height of the trail segments.
	spread_mult - radius around the bullet's path within which the segments spawn.
	neon_length_mult - length of the neon tail, length_mult if unset.
	neon_thickness_mult - thickness of the round's neon.
*/

struct bullet_trail_scaling {
	test_scene_particle_effect_id trace;
	std::vector<test_plain_missiles> rounds;
	float length_mult = 1.f;
	float thickness_mult = 1.f;
	float spread_mult = 1.f;
	std::optional<float> neon_length_mult = std::nullopt;
	std::optional<float> neon_thickness_mult = std::nullopt;

	float get_neon_length_mult() const {
		return neon_length_mult.has_value() ? *neon_length_mult : length_mult;
	}
};

inline const auto& get_bullet_trail_scalings() {
	using E = test_scene_particle_effect_id;
	using R = test_plain_missiles;

	static const auto scalings = std::vector<bullet_trail_scaling> {
		/*
			Rifles. CYAN_ROUND is shared by Bilmer2000 and Bilmik.
		*/

		{ E::BAKA47_ROUND_TRACE, { R::BAKA47_ROUND }, 0.4f, 0.7f },
		{ E::SZTURM_ROUND_TRACE, { R::SZTURM_ROUND }, 0.28f, 0.7f },
		{ E::CYAN_ROUND_TRACE, { R::CYAN_ROUND }, 0.35f },
		{ E::GALILEA_ROUND_TRACE, { R::GALILEA_ROUND }, 0.5f },

		/*
			Pistols. STEEL_PROJECTILE_TRACE is shared by Bulwark, Lews and Vindicator.
			Deagle's neon is deliberately a bit longer than its trail.
			Deagle's trail is ragged and fiery like Bulldup's - its spread about as wide as its thickest segment.
		*/

		{ E::STEEL_PROJECTILE_TRACE, { R::STEEL_ROUND, R::LEWSII_ROUND }, 0.5f },
		{ E::DEAGLE_ROUND_TRACE, { R::DEAGLE_ROUND }, 0.54f, 0.454f, 1.03f, 0.308f, 1.04f },
		{ E::ORANGE_ROUND_TRACE, { R::ORANGE_ROUND }, 0.7f, 0.8f },
		{ E::AO44_ROUND_TRACE, { R::AO44_ROUND }, 0.56f, 0.56f },
		{ E::KEK9_ROUND_TRACE, { R::KEK9_ROUND }, 0.8f },
		{ E::SN69_ROUND_TRACE, { R::PISTOL_CYAN_ROUND }, 0.8f },
		{ E::COVERT_ROUND_TRACE, { R::COVERT_CYAN_ROUND }, 0.8f },

		/*
			SMGs.
		*/

		{ E::PRO90_ROUND_TRACE, { R::PRO90_ROUND }, 0.25f },
		{ E::SZCZUR_ROUND_TRACE, { R::SZCZUR_ROUND }, 0.5f },
		{ E::ZAMIEC_ROUND_TRACE, { R::ZAMIEC_ROUND }, 0.5f },
		{ E::CYBERSPRAY_ROUND_TRACE, { R::CYBERSPRAY_ROUND }, 0.25f },

		/*
			Same length, only thinner - half through a narrower spread, half through thinner segments.
			Thinner segments widely spread look fiery, but alone get dominated by single pixel ones.
			The trail's width is about 2 * spread + the thickest segment, kept as with spread 0.275 and full segments.
			No neon tails.
			HUNTER_ROUND_TRACE also contains Bulldup's line trail,
			as it's cloned from STEEL_PROJECTILE_TRACE_PRECISE after it was added.
		*/

		{ E::STEEL_PROJECTILE_TRACE_PRECISE, {}, 1.f, 0.48f, 0.64f },
		{ E::HUNTER_ROUND_TRACE, {}, 1.f, 0.48f, 0.64f }
	};

	return scalings;
}
