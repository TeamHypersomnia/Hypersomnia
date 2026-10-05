#pragma once
#include <array>
#include "augs/graphics/rgba.h"
#include "augs/templates/maybe.h"
#include "augs/pad_bytes.h"
#include "augs/math/vec2.h"

struct randomization;

/*
	The legacy model, on maps without Physical lights: 1 / (constant + linear * d + quadratic * d^2),
	cut off where it falls to trim_alpha / 255 - with a visible edge.
*/

struct legacy_attenuation {
	// GEN INTROSPECTOR struct legacy_attenuation
	real32 constant = 0.f;
	real32 linear = 0.f;
	real32 quadratic = 0.f;

	rgba_channel trim_alpha = 3;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR

	real32 calc_reach() const;

	friend bool operator==(const legacy_attenuation&, const legacy_attenuation&) = default;
};

/*
	A light fades out from its center to exactly zero at its radius,
	following the inverse square law of a source of finite size, windowed so that it ends smoothly - see light.fsh.

	intensity scales the brightness at the center - over 1 brightens the lit textures further.
	falloff shapes the light: 0 spreads it evenly over the radius, 1 concentrates it around the center.
*/

struct light_falloff {
	// GEN INTROSPECTOR struct light_falloff
	real32 radius = 300.0f;
	real32 intensity = 1.0f;
	real32 falloff = 0.5f;
	// END GEN INTROSPECTOR

	vec2 calc_reach() const {
		return vec2::square(radius * 2);
	}

	/*
		The size of the source, relative to the radius.
	*/

	real32 calc_core() const;

	/*
		The legacy attenuation that looks the most like this falloff, for maps without Physical lights -
		fitted when the light is built, for a light color of this alpha.
	*/

	legacy_attenuation calc_legacy(real32 alpha) const;

	friend bool operator==(const light_falloff&, const light_falloff&) = default;
};

/*
	Flickering: the intensity randomly walks down from its full value, by at most magnitude / 2 of it.
*/

struct light_value_variation {
	// GEN INTROSPECTOR struct light_value_variation
	real32 magnitude = 0.f;
	real32 change_speed = 0.f;
	// END GEN INTROSPECTOR

	void update_value(randomization&, real32& val, const float dt_seconds) const;
	friend bool operator==(const light_value_variation&, const light_value_variation&) = default;

	auto& operator*=(const real32 mult) {
		magnitude *= mult;
		change_speed *= mult;

		return *this;
	}
};

namespace invariants {
	struct light {
		// GEN INTROSPECTOR struct invariants::light
		int is_new_light_attenuation = 0;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	/*
		height is in pixels of height, like shadow heights of obstacles.
		Zero means an infinitely high light, whose shadows always reach the end.
	*/

	struct light {
		// GEN INTROSPECTOR struct components::light
		light_falloff falloff;
		light_falloff wall_falloff;

		legacy_attenuation attenuation;
		legacy_attenuation wall_attenuation;

		augs::maybe<light_value_variation> variation;
		augs::maybe<light_value_variation> wall_variation;

		rgba color = white;
		bool cast_shadows = true;
		bool hue_through_walls = true;
		bool quantized = false;
		pad_bytes<1> pad;
		real32 shadow_smoothness = 1.0f;
		real32 height = 64.0f;
		// END GEN INTROSPECTOR

		light();

		vec2 calc_effective_reach() const;

		vec2 calc_reach() const;
		vec2 calc_wall_reach() const;
	};
}