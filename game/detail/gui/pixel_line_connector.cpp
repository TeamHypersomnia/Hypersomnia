#include "pixel_line_connector.h"
#include "gui/material.h"

std::vector<std::array<vec2i, 2>> get_connecting_pixel_lines(rects::ltrb<float> a, rects::ltrb<float> b) {
	using namespace augs::gui;

	vec2 ac = a.center();
	vec2 bc = b.center();

	vec2 aw2(a.w() / 2, 0);
	vec2 ah2(0, a.h() / 2);
	vec2 bw2(b.w() / 2, 0);
	vec2 bh2(0, b.h() / 2);

	if (ac.x >= b.r) {
		bool can_a_to_b = ac.y >= b.t && ac.y <= b.b;
		bool can_b_to_a = bc.y >= a.t && bc.y <= a.b;

		if (can_a_to_b && can_b_to_a) {
			if (a.h() > b.h())
				can_a_to_b = false;
			else
				can_b_to_a = false;
		}

		if (can_a_to_b) {
			return { { ac - aw2, vec2(b.r, ac.y) } };
		}
		else if (can_b_to_a) {
			return{ { bc + bw2, vec2(a.l, bc.y) } };
		}

		if (a.t >= bc.y) {
			return{ { bc + bw2, vec2(ac.x, bc.y) }, { vec2(ac.x, bc.y), vec2(ac.x, a.t) } };
		}
		else if (a.b <= bc.y) {
			return{ { bc + bw2, vec2(ac.x, bc.y) }, { vec2(ac.x, bc.y), vec2(ac.x, a.b) } };
		}
	}

	if (ac.x <= b.l) {
		bool can_a_to_b = ac.y >= b.t && ac.y <= b.b;
		bool can_b_to_a = bc.y >= a.t && bc.y <= a.b;

		if (can_a_to_b && can_b_to_a) {
			if (a.h() > b.h())
				can_a_to_b = false;
			else
				can_b_to_a = false;
		}

		if (can_a_to_b) {
			return{ { ac + aw2, vec2(b.l, ac.y) } };
		}
		else if (can_b_to_a) {
			return{ { bc - bw2, vec2(a.r, bc.y) } };
		}

		if (a.t >= bc.y) {
			return{ { bc - bw2, vec2(ac.x, bc.y) },{ vec2(ac.x, bc.y), vec2(ac.x, a.t) } };
		}
		else if (a.b <= bc.y) {
			return{ { bc - bw2, vec2(ac.x, bc.y) },{ vec2(ac.x, bc.y), vec2(ac.x, a.b) } };
		}
	}

	if (a.b < b.t) {
		bool can_a_to_b = ac.x >= b.l && ac.x <= b.r;
		bool can_b_to_a = bc.x >= a.l && bc.x <= a.r;

		if (can_a_to_b && can_b_to_a) {
			if (a.w() > b.w())
				can_a_to_b = false;
			else
				can_b_to_a = false;
		}

		if (can_a_to_b) {
			return{ { ac + ah2, vec2(ac.x, b.t) } };
		}
		else if (can_b_to_a) {
			return{ { bc - bh2, vec2(bc.x, a.b) } };
		}

		if (a.l >= bc.x) {
			return{ { bc - bh2, vec2(bc.x, ac.y) },{ vec2(bc.x, ac.y), vec2(a.l, ac.y) } };
		}
		else if (a.r <= bc.x) {
			return{ { bc - bh2, vec2(bc.x, ac.y) },{ vec2(bc.x, ac.y), vec2(a.r, ac.y) } };
		}
	}

	if (a.t > b.b) {
		bool can_a_to_b = ac.x >= b.l && ac.x <= b.r;
		bool can_b_to_a = bc.x >= a.l && bc.x <= a.r;

		if (can_a_to_b && can_b_to_a) {
			if (a.w() > b.w())
				can_a_to_b = false;
			else
				can_b_to_a = false;
		}

		if (can_a_to_b) {
			return{ { ac - ah2, vec2(ac.x, b.b) } };
		}
		else if (can_b_to_a) {
			return{ { bc + bh2, vec2(bc.x, a.t) } };
		}

		if (a.l >= bc.x) {
			return{ { bc + bh2, vec2(bc.x, ac.y) },{ vec2(bc.x, ac.y), vec2(a.l, ac.y) } };
		}
		else if (a.r <= bc.x) {
			return{ { bc + bh2, vec2(bc.x, ac.y) },{ vec2(bc.x, ac.y), vec2(a.r, ac.y) } };
		}
	}


	return { { } };
}


void draw_pixel_line_connector(rects::ltrb<float> a, rects::ltrb<float> b, augs::gui::rect::draw_info in, augs::rgba col) {
	augs::gui::material line_mat(assets::BLANK, col);

	for (auto& l : get_connecting_pixel_lines(a, b)) {
		rects::ltrb<float> line;
		
		if (l[0].x == l[1].x) {
			if (l[0].y < l[1].y) {
				line.set_position(l[0]);
				line.set_size(1, (l[1] - l[0]).y);
			}
			else {
				line.set_position(l[1]);
				line.set_size(1, (l[0] - l[1]).y);
			}
		}
		if (l[0].y == l[1].y) {
			if (l[0].x < l[1].x) {
				line.set_position(l[0]);
				line.set_size((l[1] - l[0]).x, 1);
			}
			else {
				line.set_position(l[1]);
				line.set_size((l[0] - l[1]).x, 1);
			}
		}

		augs::gui::draw_clipped_rectangle(line_mat, line, (rects::ltrb<float>*)nullptr, in.v);
	}
}