#pragma once
#include <array>
#include "augs/ensure.h"
#include "augs/math/rects.h"
#include "augs/math/vec2.h"
#include "augs/templates/container_templates.h"
#include "augs/filesystem/path.h"
#include "augs/gui/button_corner_type.h"
#include "augs/drawing/flip.h"

augs::path_type get_filename_for(const button_corner_type);

bool is_lb_complement(const button_corner_type);
bool is_button_border(const button_corner_type);
bool is_button_outside_border(const button_corner_type);
bool is_button_corner(const button_corner_type);
bool is_button_side(const button_corner_type);

template <class id_type>
struct basic_button_corners_info {
	id_type inside_texture = id_type::INVALID;
	flip_flags flip = flip_flags::make_horizontally();

	id_type get_tex_for_type(button_corner_type t) const {
		// TODO
		ensure(!flip.vertically && "Not implemented");

		if (flip.horizontally) {
			if (t == button_corner_type::LT) {
				t = button_corner_type::RT;
			}
			else if (t == button_corner_type::RT) {
				t = button_corner_type::LT;
			}
			else if (t == button_corner_type::LB) {
				t = button_corner_type::RB;
			}
			else if (t == button_corner_type::RB) {
				t = button_corner_type::LB;
			}
			else if (t == button_corner_type::L) {
				t = button_corner_type::R;
			}
			else if (t == button_corner_type::R) {
				t = button_corner_type::L;
			}


			if (t == button_corner_type::LT_BORDER) {
				t = button_corner_type::RT_BORDER;
			}
			else if (t == button_corner_type::RT_BORDER) {
				t = button_corner_type::LT_BORDER;
			}
			else if (t == button_corner_type::LB_BORDER) {
				t = button_corner_type::RB_BORDER;
			}
			else if (t == button_corner_type::RB_BORDER) {
				t = button_corner_type::LB_BORDER;
			}
			else if (t == button_corner_type::L_BORDER) {
				t = button_corner_type::R_BORDER;
			}
			else if (t == button_corner_type::R_BORDER) {
				t = button_corner_type::L_BORDER;
			}


			if (t == button_corner_type::LT_INTERNAL_BORDER) {
				t = button_corner_type::RT_INTERNAL_BORDER;
			}
			else if (t == button_corner_type::RT_INTERNAL_BORDER) {
				t = button_corner_type::LT_INTERNAL_BORDER;
			}
			else if (t == button_corner_type::LB_INTERNAL_BORDER) {
				t = button_corner_type::RB_INTERNAL_BORDER;
			}
			else if (t == button_corner_type::RB_INTERNAL_BORDER) {
				t = button_corner_type::LB_INTERNAL_BORDER;
			}
		}

		switch (t) {

		case button_corner_type::INSIDE:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 0);
			break;

		case button_corner_type::LT:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 1);
			break;
		case button_corner_type::RT:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 2);
			break;
		case button_corner_type::RB:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 3);
			break;
		case button_corner_type::LB:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 4);
			break;

		case button_corner_type::L:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 5);
			break;
		case button_corner_type::T:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 6);
			break;
		case button_corner_type::R:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 7);
			break;
		case button_corner_type::B:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 8);
			break;

		case button_corner_type::LB_COMPLEMENT:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 9);
			break;

		case button_corner_type::LT_BORDER:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 10);
			break;
		case button_corner_type::RT_BORDER:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 11);
			break;
		case button_corner_type::RB_BORDER:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 12);
			break;
		case button_corner_type::LB_BORDER:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 13);
			break;


		case button_corner_type::L_BORDER:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 14);
			break;
		case button_corner_type::T_BORDER:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 15);
			break;
		case button_corner_type::R_BORDER:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 16);
			break;
		case button_corner_type::B_BORDER:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 17);
			break;

		case button_corner_type::LB_COMPLEMENT_BORDER:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 18);
			break;

		case button_corner_type::LT_INTERNAL_BORDER:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 19);
			break;
		case button_corner_type::RT_INTERNAL_BORDER:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 20);
			break;
		case button_corner_type::RB_INTERNAL_BORDER:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 21);
			break;
		case button_corner_type::LB_INTERNAL_BORDER:
			return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 22);
			break;

		default: ensure(false); return static_cast<id_type>(static_cast<unsigned>(inside_texture) + 0); break;
		}
	}

	template <class M>
	auto size_of(const M& manager, const button_corner_type t) const {
		return manager.at(get_tex_for_type(t)).get_original_size();
	}

	template <class M>
	ltrb cornered_rc_to_internal_rc(const M& manager, ltrb l) const {
		l.l += size_of(manager, button_corner_type::L).x;
		l.t += size_of(manager, button_corner_type::T).y;
		l.r -= size_of(manager, button_corner_type::R).x;
		l.b -= size_of(manager, button_corner_type::B).y;
		return l;
	}

	template <class M>
	ltrb internal_rc_to_cornered_rc(const M& manager, ltrb l) const {
		l.l -= size_of(manager, button_corner_type::L).x;
		l.t -= size_of(manager, button_corner_type::T).y;
		l.r += size_of(manager, button_corner_type::R).x;
		l.b += size_of(manager, button_corner_type::B).y;
		return l;
	}

	template <class M>
	vec2i cornered_size_to_internal_size(const M& manager, vec2i l) const {
		l.x -= size_of(manager, button_corner_type::L).x;
		l.x -= size_of(manager, button_corner_type::R).x;
		l.y -= size_of(manager, button_corner_type::T).y;
		l.y -= size_of(manager, button_corner_type::B).y;
		return l;
	}

	template <class M>
	vec2i internal_size_to_cornered_size(const M& manager, vec2i l) const {
		l.x += size_of(manager, button_corner_type::L).x;
		l.x += size_of(manager, button_corner_type::R).x;
		l.y += size_of(manager, button_corner_type::T).y;
		l.y += size_of(manager, button_corner_type::B).y;
		return l;
	}

	template <class M, class L>
	void for_each_button_corner(const M& necessarys, const ltrb origin, L callback) const {
		for (auto i = button_corner_type::INSIDE; i < button_corner_type::COUNT; i = static_cast<button_corner_type>(static_cast<int>(i) + 1)) {
			const auto tex_id = get_tex_for_type(i);
			const auto tex = necessarys.at(tex_id);

			static_assert(!has_member_find_v<M, id_type>, "Assuming that the id is always found.");

			if (!tex.exists()) {
				continue;
			}

			const vec2 s = tex.get_original_size();

			ltrb target_rect;
			
			if (i == button_corner_type::INSIDE) {
				target_rect = origin;
			}

			else if (i == button_corner_type::LT || i == button_corner_type::LT_BORDER || i == button_corner_type::LT_INTERNAL_BORDER) {
				target_rect.set_size(s);
				target_rect.set_position(origin.left_top() - s);
			}
			
			else if (i == button_corner_type::RT || i == button_corner_type::RT_BORDER || i == button_corner_type::RT_INTERNAL_BORDER) {
				target_rect.set_size(s);
				target_rect.set_position(origin.right_top() - vec2(0, s.y));
			}

			else if (i == button_corner_type::RB || i == button_corner_type::RB_BORDER || i == button_corner_type::RB_INTERNAL_BORDER) {
				target_rect.set_size(s);
				target_rect.set_position(origin.right_bottom());
			}
			
			else if (i == button_corner_type::LB || i == button_corner_type::LB_BORDER || i == button_corner_type::LB_INTERNAL_BORDER) {
				target_rect.set_size(s);
				target_rect.set_position(origin.left_bottom() - vec2(s.x, 0));
			}


			else if (i == button_corner_type::L || i == button_corner_type::L_BORDER) {
				target_rect.set_size({ s.x, origin.h() });
				target_rect.set_position(origin.left_top() - vec2(s.x, 0));
			}

			else if (i == button_corner_type::T || i == button_corner_type::T_BORDER) {
				target_rect.set_size({ origin.w(), s.y });
				target_rect.set_position(origin.left_top() - vec2(0, s.y));
			}

			else if (i == button_corner_type::R || i == button_corner_type::R_BORDER) {
				target_rect.set_size({ s.x, origin.h() });
				target_rect.set_position(origin.right_top());
			}

			else if (i == button_corner_type::B || i == button_corner_type::B_BORDER) {
				target_rect.set_size({ origin.w(), s.y });
				target_rect.set_position(origin.left_bottom());
			}


			else if (i == button_corner_type::LB_COMPLEMENT || i == button_corner_type::LB_COMPLEMENT_BORDER) {
				target_rect.set_size(s);

				if (flip.horizontally) {
					target_rect.set_position(origin.right_bottom());
				}
				else {
					target_rect.set_position(origin.left_bottom() - vec2(s.x, 0));
				}
			}
			else {
				ensure("Unsupported button border type" && false);
			}

			callback(i, tex_id, target_rect);
		}
	}
};