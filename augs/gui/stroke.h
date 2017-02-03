#pragma once
#include "material.h"

namespace augs {
	namespace gui {
		struct solid_stroke {
			struct border {
				int width;
				material mat;
			} left, top, right, bottom;

			enum type {
				INSIDE, OUTSIDE
			} _type;

			solid_stroke(int width = 1, const material& = material(), type = OUTSIDE);

			void set_width(int);
			void set_material(const material&);

			void draw(
				std::vector<augs::vertex_triangle>& out, 
				ltrb origin, 
				const ltrb clipper = ltrb(),
				const int border_spacing = 0
			) const;
		};
	}
}
