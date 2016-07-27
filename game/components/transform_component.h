#pragma once
#include "augs/math/rects.h"
#include "augs/math/vec2.h"

namespace components {
	struct transform {
		vec2 pos;
		float rotation = 0.0f;

		struct previous_state {
			vec2 pos;
			float rotation = 0.0f;

			template <class Archive>
			void serialize(Archive& ar) {
				ar(
					CEREAL_NVP(pos),
					CEREAL_NVP(rotation)
				);
			}
		} previous;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(previous),
				CEREAL_NVP(pos),
				CEREAL_NVP(rotation)
			);
		}
		
		transform(float x, float y, float rotation = 0.0f);
		transform(vec2 pos = vec2(), float rotation = 0.0f);
		transform(previous_state state);

		transform operator+(const transform& b) const;
		transform operator-(const transform& b) const;
		transform& operator+=(const transform& b);
		bool operator==(const transform& b) const;

		transform interpolated(float ratio, float epsilon = 1.f) const;
		void flip_rotation();
		void reset();
		vec2 interpolation_direction() const;
	};
}

namespace augs {
	template <class A>
	components::transform interp(components::transform a, components::transform b, A alpha) {
		components::transform res;
		res.pos = augs::interp(a.pos, b.pos, alpha);
		res.rotation = augs::interp(vec2().set_from_degrees(a.rotation), vec2().set_from_degrees(b.rotation), alpha).degrees();
		return res;
	}
}

#include "game/detail/position_scripts.h"
