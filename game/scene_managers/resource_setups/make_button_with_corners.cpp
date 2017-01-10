#include "all.h"
#include "augs/gui/button_corners.h"

namespace resource_setups {
	void make_button_with_corners(
		resources::manager& manager,
		const assets::texture_id inside_tex,

		const rgba border_color,
		const rgba inside_color,

		const int lower_side,
		const int upper_side,

		const int inside_border_padding,
		const bool make_lb_complement
	) {

		button_corners_info corners;
		corners.flip_horizontally = false;
		corners.lt_texture = static_cast<assets::texture_id>(static_cast<size_t>(inside_tex) + 1);

		button_corners_info borders_corners;
		borders_corners.flip_horizontally = false;
		borders_corners.lt_texture = static_cast<assets::texture_id>(static_cast<size_t>(inside_tex) + 1 + 8 + 1);

		auto tex = [&](const auto t) {
			return corners.get_tex_for_type(t);
		};

		auto borders_tex = [&](const auto t) {
			return borders_corners.get_tex_for_type(t);
		};

		{
			augs::image l;
			augs::image t;
			augs::image r;
			augs::image b;

			l.create(lower_side, 1, 4);
			l.paint_line({ 1, 0 }, { lower_side - 1, 0 }, inside_color);

			r.create(upper_side, 1, 4);
			r.paint_line({ upper_side - 2, 0 }, { 0, 0 }, inside_color);

			b.create(1, lower_side, 4);
			b.paint_line({ 0, lower_side - 2 }, { 0, 0 }, inside_color);

			t.create(1, upper_side, 4);
			t.paint_line({ 0, 1 }, { 0, upper_side - 1 }, inside_color);

			manager.create(tex(button_corner_type::L), l);
			manager.create(tex(button_corner_type::T), t);
			manager.create(tex(button_corner_type::R), r);
			manager.create(tex(button_corner_type::B), b);
		}

		{
			augs::image lt;
			augs::image rt;
			augs::image rb;
			augs::image lb;

			augs::image lb_complement;
			augs::image lb_complement_border;

			lt.create(lower_side, upper_side, 4);
			lt.fill(inside_color);
			lt.paint_line({ 0, 0 }, { lower_side - 1, 0 }, { 0, 0, 0, 0 });
			lt.paint_line({ 0, 0 }, { 0, upper_side - 1 }, { 0, 0, 0, 0 });

			rt.create(upper_side, upper_side, 4);
			rt.fill({ 0, 0, 0, 0 });

			for (int i = 1; i < upper_side; ++i) {
				rt.paint_line({ 0, i }, { upper_side - i - 1, upper_side - 1 }, inside_color);
			}

			rb.create(upper_side, lower_side, 4);
			rb.fill(inside_color);
			rb.paint_line({ upper_side - 1, lower_side - 1 }, { 0, lower_side - 1 }, { 0, 0, 0, 0 });
			rb.paint_line({ upper_side - 1, lower_side - 1 }, { upper_side - 1, 0 }, { 0, 0, 0, 0 });

			lb.create(lower_side, lower_side, 4);
			lb.fill({ 0, 0, 0, 0 });

			for (int i = 1; i < lower_side; ++i) {
				lb.paint_line({ i, 0 }, { lower_side - 1, lower_side - 1 - i }, inside_color);
			}

			lb_complement.create(lower_side, lower_side, 4);

			for (int i = 1; i < lower_side; ++i) {
				lb_complement.paint_line({ 0, i }, { lower_side - 1 - i, lower_side - 1 }, inside_color);
			}

			manager.create(tex(button_corner_type::LT), lt);
			manager.create(tex(button_corner_type::RT), rt);
			manager.create(tex(button_corner_type::RB), rb);
			manager.create(tex(button_corner_type::LB), lb);

			if (make_lb_complement) {
				manager.create(tex(button_corner_type::LB_COMPLEMENT), lb_complement);
			}
		}

		{
			augs::image l;
			augs::image t;
			augs::image r;
			augs::image b;

			l.create(lower_side, 1, 4);
			l.set_pixel({ 0, 0 }, border_color);

			r.create(upper_side, 1, 4);
			r.set_pixel({ upper_side - 1, 0 }, border_color);

			b.create(1, lower_side, 4);
			b.set_pixel({ 0, lower_side - 1 }, border_color);

			t.create(1, upper_side, 4);
			t.set_pixel({ 0, 0 }, border_color);

			manager.create(borders_tex(button_corner_type::L), l);
			manager.create(borders_tex(button_corner_type::T), t);
			manager.create(borders_tex(button_corner_type::R), r);
			manager.create(borders_tex(button_corner_type::B), b);
		}

		{
			augs::image lt;
			augs::image rt;
			augs::image rb;
			augs::image lb;

			augs::image lb_complement;

			lt.create(lower_side, upper_side, 4);
			lt.paint_line({ 0, 0 }, { 0, upper_side - 1 }, border_color);
			lt.paint_line({ 0, 0 }, { lower_side - 1, 0 }, border_color);

			lt.paint_line({ inside_border_padding, inside_border_padding }, { inside_border_padding, upper_side - 1 }, border_color);
			lt.paint_line({ inside_border_padding, inside_border_padding }, { lower_side - 1, inside_border_padding }, border_color);

			rt.create(upper_side, upper_side, 4);
			rt.fill({ 0, 0, 0, 0 });

			rt.paint_line({ 0, 0 }, { upper_side - 1, upper_side - 1 }, border_color);
			rt.paint_line({ 0, inside_border_padding }, { upper_side - 1 - inside_border_padding, upper_side - 1 }, border_color);

			rb.create(upper_side, lower_side, 4);
			rb.paint_line({ upper_side - 1, lower_side - 1 }, { 0, lower_side - 1 }, border_color);
			rb.paint_line({ upper_side - 1, lower_side - 1 }, { upper_side - 1, 0 }, border_color);

			rb.paint_line({ upper_side - 1 - inside_border_padding, lower_side - 1 - inside_border_padding }, { 0, lower_side - 1 - inside_border_padding }, border_color);
			rb.paint_line({ upper_side - 1 - inside_border_padding, lower_side - 1 - inside_border_padding }, { upper_side - 1 - inside_border_padding, 0 }, border_color);

			lb.create(lower_side, lower_side, 4);
			lb.fill({ 0, 0, 0, 0 });

			lb.paint_line({ 0, 0 }, { lower_side - 1, lower_side - 1 }, border_color);
			lb.paint_line({ inside_border_padding, 0 }, { lower_side - 1, lower_side - 1 - inside_border_padding }, border_color);

			lb_complement.create(lower_side, lower_side, 4);
			lb_complement.paint_line({ 0, lower_side - 1 }, { lower_side - 1, lower_side - 1 }, border_color);
			lb_complement.paint_line({ 0, lower_side - 1 }, { 0, 0 }, border_color);

			manager.create(borders_tex(button_corner_type::LT), lt);
			manager.create(borders_tex(button_corner_type::RT), rt);
			manager.create(borders_tex(button_corner_type::RB), rb);
			manager.create(borders_tex(button_corner_type::LB), lb);

			if (make_lb_complement) {
				manager.create(borders_tex(button_corner_type::LB_COMPLEMENT), lb_complement);
			}
		}

		augs::image inside;
		inside.create(100, 100, 4);
		inside.fill(inside_color);
		manager.create(inside_tex, inside);
	}
}