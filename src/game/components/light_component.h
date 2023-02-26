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
	friend bool operator==(const light_value_variation&, const light_value_variation&) = default;

	auto& operator*=(const atten_t& mult) {
		magnitude *= mult;
		change_speed *= mult;

		return *this;
	}
};

struct attenuation_variations {
	// GEN INTROSPECTOR struct attenuation_variations
	light_value_variation constant;
	light_value_variation linear;
	light_value_variation quadratic;
	// END GEN INTROSPECTOR

	auto& operator*=(const atten_t& mult) {
		constant *= mult;
		linear *= mult;
		quadratic *= mult;

		return *this;
	}
};

struct attenuation_properties {
	// GEN INTROSPECTOR struct attenuation_properties
	atten_t constant = 0.f;
	atten_t linear = 0.f;
	atten_t quadratic = 0.f;

	rgba_channel trim_alpha = 3;
	pad_bytes<3> pad;

	augs::maybe<vec2> trim_reach = augs::maybe<vec2>(vec2::square(300.f), false);
	// END GEN INTROSPECTOR

	real32 calc_reach() const;
	vec2 calc_reach_trimmed() const;
};

namespace invariants {
	struct light {
		// GEN INTROSPECTOR struct invariants::light
		int is_new_light_attenuation = 0;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct light {
		// GEN INTROSPECTOR struct components::light
		attenuation_properties attenuation;
		attenuation_properties wall_attenuation;

		augs::maybe<attenuation_variations> variation;
		augs::maybe<attenuation_variations> wall_variation;
		augs::maybe<std::array<light_value_variation, 2>> position_variations;

		rgba color = white;
		// END GEN INTROSPECTOR

		light();

		vec2 calc_effective_reach() const;

		vec2 calc_reach_trimmed() const;
		vec2 calc_wall_reach_trimmed() const;
	};
}