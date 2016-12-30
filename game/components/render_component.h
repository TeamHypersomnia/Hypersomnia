#pragma once
#include "transform_component.h"

#include "augs/math/vec2.h"
#include "game/enums/render_layer.h"
#include "augs/graphics/pixel.h"

#include "augs/padding_byte.h"

namespace components {
	struct render {
		bool screen_space_transform = false;
		bool draw_border = false;
		render_layer layer = render_layer::INVALID;
		padding_byte pad;

		augs::rgba border_color;

		unsigned last_step_when_visible = 0;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(interpolate),
				CEREAL_NVP(screen_space_transform),
				CEREAL_NVP(draw_border),

				CEREAL_NVP(layer),

				CEREAL_NVP(border_color),

				CEREAL_NVP(last_step_when_visible)
			);
		}
	};
}