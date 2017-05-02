#include "stroke.h"
#include "rect.h"

namespace augs {
	namespace gui {
		solid_stroke::solid_stroke(int width, const material& mat, type _type) : _type(_type) {
			left.width = width; left.mat = mat;
			right = top = bottom = left;
		}

		void solid_stroke::set_width(int w) {
			left.width = top.width = right.width = bottom.width = w;
		}

		void solid_stroke::set_material(const material& mat) {
			left.mat = top.mat = right.mat = bottom.mat = mat;
		}

		void solid_stroke::draw(
			std::vector<augs::vertex_triangle>& out, 
			ltrb g, 
			const ltrb clipper,
			const int border_spacing
		) const {
			if (_type == OUTSIDE) {
				g.l -= left.width + border_spacing;
				g.t -= top.width + border_spacing;
				g.r += right.width + border_spacing;
				g.b += bottom.width + border_spacing;
			}

			if (border_spacing == 0) {
				g.l++;
				g.t++;
			}

			ltrb lines[4] = { g, g, g, g };

			lines[0].r = g.l + left.width;
			lines[1].b = g.t + top.width;
			lines[2].l = g.r - right.width;
			lines[3].t = g.b - bottom.width;

			if (top.width) {
				lines[0].t += top.width;
				lines[2].t += top.width;
			}
			if (bottom.width) {
				lines[0].b -= top.width;
				lines[2].b -= top.width;
			}

			draw_clipped_rect(left.mat, lines[0], clipper, out);
			draw_clipped_rect(top.mat, lines[1], clipper, out);
			draw_clipped_rect(right.mat, lines[2], clipper, out);
			draw_clipped_rect(bottom.mat, lines[3], clipper, out);
		}
	}
}