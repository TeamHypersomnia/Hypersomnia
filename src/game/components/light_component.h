#pragma once
#include <array>
#include "augs/graphics/rgba.h"
#include "augs/templates/maybe.h"
#include "augs/pad_bytes.h"

struct randomization;

using atten_t = real32;

struct light_value_variation {
	// GEN INTROSPECTOR struct light_value_variation
	atten_t magnitude = 0.f;
	atten_t change_speed = 0.f;
	// END GEN INTROSPECTOR

	void update_value(randomization&, atten_t& val, const float dt_seconds) const;
};

struct attenuation_variations {
	// GEN INTROSPECTOR struct attenuation_variations
	light_value_variation constant;
	light_value_variation linear;
	light_value_variation quadratic;
	// END GEN INTROSPECTOR
};

struct attenuation_properties {
	// GEN INTROSPECTOR struct attenuation_properties
	atten_t constant = 0.f;
	atten_t linear = 0.f;
	atten_t quadratic = 0.f;

	rgba_channel trim_alpha = 3;
	pad_bytes<3> pad;

	augs::maybe<real32> trim_reach = augs::maybe<real32>(300.f, false);
	// END GEN INTROSPECTOR

	real32 calc_reach() const;
	real32 calc_reach_trimmed() const;

	void add_max(const attenuation_variations&);
};

namespace invariants {
	struct light {
		// GEN INTROSPECTOR struct invariants::light
		attenuation_properties attenuation;
		attenuation_properties wall_attenuation;

		augs::maybe<attenuation_variations> variation;
		augs::maybe<attenuation_variations> wall_variation;
		augs::maybe<std::array<light_value_variation, 2>> position_variations;
		// END GEN INTROSPECTOR

		light();

		real32 calc_reach_trimmed() const;
		real32 calc_wall_reach_trimmed() const;
	};
}

namespace components {
	struct light {
		// GEN INTROSPECTOR struct components::light
		rgba color = white;
		// END GEN INTROSPECTOR
	};
}