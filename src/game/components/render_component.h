#pragma once
#include <cstdint>
#include "transform_component.h"
#include "augs/math/vec2.h"
#include "game/enums/render_layer.h"
#include "augs/graphics/rgba.h"
#include "augs/pad_bytes.h"
#include "augs/misc/enum/enum_boolset.h"
#include "game/detail/special_render_function.h"

namespace invariants {
	/*
		shadow_height is in pixels of height, which sun_shadow_settings::step turns into the length of sun shadows.
		reaches_ceiling makes point lights never shine over the obstacle, whatever its shadow height.
		Both are read only for entities with physical bodies.

		casts_foreground_shadow makes a sprite cast its silhouette as a sun shadow.
		Foreground ones always cast from above characters - foreground_shadow_extra_height only adds to that.
		For anything lower, foreground_shadow_extra_height is the whole height,
		and such sprites are drawn above decals and corpses, shadowed only by what's taller.
		foreground_shadow_opacity scales the arena's shadow strength for it - foliage lets some light through.

		casts_overlay_shadow draws a translucent shadow under a solid obstacle lying around, like a detached head.
	*/

	struct render {
		// GEN INTROSPECTOR struct invariants::render
		render_layer layer = render_layer::GROUND;
		augs::enum_boolset<special_render_function> special_functions;
		uint8_t shadow_height = 32;
		bool reaches_ceiling = false;
		bool casts_foreground_shadow = false;
		uint8_t foreground_shadow_extra_height = 0;
		uint8_t foreground_shadow_opacity = 153;
		bool casts_overlay_shadow = false;
		pad_bytes<2> pad;
		// END GEN INTROSPECTOR
	};
}