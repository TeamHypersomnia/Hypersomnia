#include "all.h"
#include "augs/gui/button_corners.h"

namespace resource_setups {
	void make_button_with_corners(
		resources::manager& manager,
		const assets::texture_id inside_tex,

		const rgba border_color,
		const rgba inside_color,

		const unsigned lower_side,
		const unsigned upper_side,

		const unsigned inside_border_padding,
		const bool make_lb_complement
	) {

		button_corners_info corners;
		corners.flip_horizontally = false;
		corners.lt_texture = static_cast<assets::texture_id>(static_cast<size_t>(inside_tex) + 1);

		auto tex = [&](const auto t) {
			return corners.get_tex_for_type(t);
		};

		{
			augs::image l;
			augs::image t;
			augs::image r;
			augs::image b;

			l.create(lower_side, 1u);
			l.paint_line({ 1, 0 }, { lower_side - 1, 0 }, inside_color);

			r.create(upper_side, 1u);
			r.paint_line({ upper_side - 2, 0 }, { 0, 0 }, inside_color);

			b.create(1u, lower_side);
			b.paint_line({ 0, lower_side - 2 }, { 0, 0 }, inside_color);

			t.create(1u, upper_side);
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

			lt.create(lower_side, upper_side);
			lt.fill(inside_color);
			lt.paint_line({ 0, 0 }, { lower_side - 1, 0 }, { 0, 0, 0, 0 });
			lt.paint_line({ 0, 0 }, { 0, upper_side - 1 }, { 0, 0, 0, 0 });

			rt.create(upper_side, upper_side);
			rt.fill({ 0, 0, 0, 0 });

			for (unsigned i = 1; i < upper_side; ++i) {
				rt.paint_line({ 0, i }, { upper_side - i - 1, upper_side - 1 }, inside_color);
			}

			rb.create(upper_side, lower_side);
			rb.fill(inside_color);
			rb.paint_line({ upper_side - 1, lower_side - 1 }, { 0, lower_side - 1 }, { 0, 0, 0, 0 });
			rb.paint_line({ upper_side - 1, lower_side - 1 }, { upper_side - 1, 0 }, { 0, 0, 0, 0 });

			lb.create(lower_side, lower_side);
			lb.fill({ 0, 0, 0, 0 });

			for (unsigned i = 1; i < lower_side; ++i) {
				lb.paint_line({ i, 0 }, { lower_side - 1, lower_side - 1 - i }, inside_color);
			}

			lb_complement.create(lower_side, lower_side);

			for (unsigned i = 1; i < lower_side; ++i) {
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

			l.create(lower_side, 1u);
			l.pixel({ 0, 0 }) = border_color;

			r.create(upper_side, 1u);
			r.pixel({ upper_side - 1, 0 }) = border_color;

			b.create(1u, lower_side);
			b.pixel({ 0, lower_side - 1 }) = border_color;

			t.create(1u, upper_side);
			t.pixel({ 0, 0 }) = border_color;

			manager.create(tex(button_corner_type::L_BORDER), l);
			manager.create(tex(button_corner_type::T_BORDER), t);
			manager.create(tex(button_corner_type::R_BORDER), r);
			manager.create(tex(button_corner_type::B_BORDER), b);
		}

		{
			augs::image lt;
			augs::image rt;
			augs::image rb;
			augs::image lb;

			augs::image lb_complement;

			lt.create(lower_side, upper_side);
			lt.paint_line({ 0, 0 }, { 0, upper_side - 1 }, border_color);
			lt.paint_line({ 0, 0 }, { lower_side - 1, 0 }, border_color);

			rt.create(upper_side, upper_side);
			rt.fill({ 0, 0, 0, 0 });

			rt.paint_line({ 0, 0 }, { upper_side - 1, upper_side - 1 }, border_color);

			rb.create(upper_side, lower_side);
			rb.paint_line({ upper_side - 1, lower_side - 1 }, { 0, lower_side - 1 }, border_color);
			rb.paint_line({ upper_side - 1, lower_side - 1 }, { upper_side - 1, 0 }, border_color);

			lb.create(lower_side, lower_side);
			lb.fill({ 0, 0, 0, 0 });

			lb.paint_line({ 0, 0 }, { lower_side - 1, lower_side - 1 }, border_color);

			lb_complement.create(lower_side, lower_side);
			lb_complement.paint_line({ 0, lower_side - 1 }, { lower_side - 1, lower_side - 1 }, border_color);
			lb_complement.paint_line({ 0, lower_side - 1 }, { 0, 0 }, border_color);

			manager.create(tex(button_corner_type::LT_BORDER), lt);
			manager.create(tex(button_corner_type::RT_BORDER), rt);
			manager.create(tex(button_corner_type::RB_BORDER), rb);
			manager.create(tex(button_corner_type::LB_BORDER), lb);

			if (make_lb_complement) {
				manager.create(tex(button_corner_type::LB_COMPLEMENT_BORDER), lb_complement);
			}
		}

		{
			augs::image lt;
			augs::image rt;
			augs::image rb;
			augs::image lb;

			augs::image lb_complement;

			lt.create(lower_side, upper_side);
			lt.paint_line({ inside_border_padding, inside_border_padding }, { inside_border_padding, upper_side - 1 }, border_color);
			lt.paint_line({ inside_border_padding, inside_border_padding }, { lower_side - 1, inside_border_padding }, border_color);

			rt.create(upper_side, upper_side);
			rt.fill({ 0, 0, 0, 0 });
			rt.paint_line({ 0, inside_border_padding }, { upper_side - 1 - inside_border_padding, upper_side - 1 }, border_color);

			rb.create(upper_side, lower_side);
			rb.paint_line({ upper_side - 1 - inside_border_padding, lower_side - 1 - inside_border_padding }, { 0, lower_side - 1 - inside_border_padding }, border_color);
			rb.paint_line({ upper_side - 1 - inside_border_padding, lower_side - 1 - inside_border_padding }, { upper_side - 1 - inside_border_padding, 0 }, border_color);

			lb.create(lower_side, lower_side);
			lb.fill({ 0, 0, 0, 0 });
			lb.paint_line({ inside_border_padding, 0 }, { lower_side - 1, lower_side - 1 - inside_border_padding }, border_color);

			manager.create(tex(button_corner_type::LT_INTERNAL_BORDER), lt);
			manager.create(tex(button_corner_type::RT_INTERNAL_BORDER), rt);
			manager.create(tex(button_corner_type::RB_INTERNAL_BORDER), rb);
			manager.create(tex(button_corner_type::LB_INTERNAL_BORDER), lb);
		}

		augs::image inside;
		inside.create(100u, 100u);
		inside.fill(inside_color);
		manager.create(inside_tex, inside);
	}
}