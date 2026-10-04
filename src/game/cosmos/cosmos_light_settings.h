#pragma once
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"

/*
	step is the displacement of an environment shadow per one pixel of the caster's shadow height.
	strength is the fraction of the ambient light removed inside the shadow.
	smoothness is how much of the strength a shadow loses along its length, fading linearly - 1 fades it out completely.
	hue_preservation blends between removing the ambient color (0) and only its brightness (1).
*/

struct sun_shadow_settings {
	// GEN INTROSPECTOR struct sun_shadow_settings
	vec2 step = vec2(1.87f, 2.34f);
	real32 strength = 0.35f;
	real32 smoothness = 1.0f;
	real32 hue_preservation = 1.0f;
	// END GEN INTROSPECTOR

	bool operator==(const sun_shadow_settings&) const = default;
};

/*
	smoothness_mult scales the shadow smoothness of every point light.
	hue_preservation blends between shadows of point lights taking the hue of the remaining light (0)
	and keeping the hue of the light they would get without the shadow, only darker (1).
*/

struct point_light_shadow_settings {
	// GEN INTROSPECTOR struct point_light_shadow_settings
	real32 smoothness_mult = 0.5f;
	real32 hue_preservation = 1.0f;
	// END GEN INTROSPECTOR

	bool operator==(const point_light_shadow_settings&) const = default;
};

/*
	physical_lights chooses the falloff of point lights: the physical one (light_falloff) or the legacy one (legacy_attenuation).

	brighten_lights brightens lit pixels by up to twice as much, the more the more intense the light -
	so that a lit texture can reach its full brightness under a light dimmer than white.
	With physical lights smoothly, with the legacy ones in a few discrete bands.
	Without it, the light is applied as it is.
*/

struct cosmos_light_settings {
	// GEN INTROSPECTOR struct cosmos_light_settings
	rgba ambient_color = rgba(53, 97, 102, 255);
	bool physical_lights = true;
	bool brighten_lights = true;
	sun_shadow_settings sun_shadows;
	point_light_shadow_settings point_light_shadows;
	// END GEN INTROSPECTOR
};
