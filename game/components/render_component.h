#pragma once
#include "transform_component.h"

#include "augs/math/vec2.h"
#include "game/enums/render_layer.h"
#include "augs/graphics/pixel.h"

namespace components {
	struct render {
		transform previous_transform;
		transform saved_actual_transform;

		bool interpolate = true;
		bool snap_interpolation_when_close = true;

		render_layer layer = render_layer::INVALID;

		bool absolute_transform = false;

		bool draw_border = false;
		augs::rgba border_color;

		unsigned last_step_when_visible = 0;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(previous_transform),
				CEREAL_NVP(saved_actual_transform),

				CEREAL_NVP(interpolate),
				CEREAL_NVP(snap_interpolation_when_close),

				CEREAL_NVP(layer),

				CEREAL_NVP(absolute_transform),

				CEREAL_NVP(draw_border),
				CEREAL_NVP(border_color),

				CEREAL_NVP(last_step_when_visible),
			);
		}

		vec2 interpolation_direction() const {
			return saved_actual_transform.pos - previous_transform.pos;
		}
	};
}