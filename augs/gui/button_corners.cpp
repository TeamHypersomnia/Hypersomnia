#include "button_corners.h"

using namespace assets;

bool is_button_corner(const button_corner_type t) {
	if (
		t == button_corner_type::LT
		|| t == button_corner_type::RT
		|| t == button_corner_type::RB
		|| t == button_corner_type::LB
	) {
		return true;
	}

	return false;
}

bool is_button_side(const button_corner_type t) {
	if (
		t == button_corner_type::L
		|| t == button_corner_type::T
		|| t == button_corner_type::R
		|| t == button_corner_type::B
		) {
		return true;
	}

	return false;
}

texture_id button_corners_info::get_tex_for_type(button_corner_type t) const {
	if (flip_horizontally) {
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
	}

	switch (t) {
	case button_corner_type::LT:
		return static_cast<texture_id>(static_cast<unsigned>(lt_texture) + 0);
		break;
	case button_corner_type::RT:
		return static_cast<texture_id>(static_cast<unsigned>(lt_texture) + 1);
		break;
	case button_corner_type::RB:
		return static_cast<texture_id>(static_cast<unsigned>(lt_texture) + 2);
		break;
	case button_corner_type::LB:
		return static_cast<texture_id>(static_cast<unsigned>(lt_texture) + 3);
		break;

	case button_corner_type::L:
		return static_cast<texture_id>(static_cast<unsigned>(lt_texture) + 4);
		break;
	case button_corner_type::T:
		return static_cast<texture_id>(static_cast<unsigned>(lt_texture) + 5);
		break;
	case button_corner_type::R:
		return static_cast<texture_id>(static_cast<unsigned>(lt_texture) + 6);
		break;
	case button_corner_type::B:
		return static_cast<texture_id>(static_cast<unsigned>(lt_texture) + 7);
		break;

	case button_corner_type::LB_COMPLEMENT:
		return static_cast<texture_id>(static_cast<unsigned>(lt_texture) + 8);
		break;

	default: ensure(false); return static_cast<texture_id>(static_cast<unsigned>(lt_texture) + 0); break;
	}
}

ltrb button_corners_info::cornered_rc_to_internal_rc(ltrb l) const {
	auto& manager = get_resource_manager();
	l.l += manager.find(get_tex_for_type(button_corner_type::L))->tex.get_size().x;
	l.t += manager.find(get_tex_for_type(button_corner_type::T))->tex.get_size().y;
	l.r -= manager.find(get_tex_for_type(button_corner_type::R))->tex.get_size().x;
	l.b -= manager.find(get_tex_for_type(button_corner_type::B))->tex.get_size().y;
	return l;
}

ltrb button_corners_info::internal_rc_to_cornered_rc(ltrb l) const {
	auto& manager = get_resource_manager();
	l.l += manager.find(get_tex_for_type(button_corner_type::L))->tex.get_size().x;
	l.t += manager.find(get_tex_for_type(button_corner_type::T))->tex.get_size().y;
	l.r -= manager.find(get_tex_for_type(button_corner_type::R))->tex.get_size().x;
	l.b -= manager.find(get_tex_for_type(button_corner_type::B))->tex.get_size().y;
	return l;
}

vec2i button_corners_info::cornered_size_to_internal_size(vec2i l) const {
	auto& manager = get_resource_manager();
	l.x -= manager.find(get_tex_for_type(button_corner_type::L))->tex.get_size().x;
	l.x -= manager.find(get_tex_for_type(button_corner_type::R))->tex.get_size().x;
	l.y -= manager.find(get_tex_for_type(button_corner_type::T))->tex.get_size().y;
	l.y -= manager.find(get_tex_for_type(button_corner_type::B))->tex.get_size().y;
	return l;
}

vec2i button_corners_info::internal_size_to_cornered_size(vec2i l) const {
	auto& manager = get_resource_manager();
	l.x += manager.find(get_tex_for_type(button_corner_type::L))->tex.get_size().x;
	l.x += manager.find(get_tex_for_type(button_corner_type::R))->tex.get_size().x;
	l.y += manager.find(get_tex_for_type(button_corner_type::T))->tex.get_size().y;
	l.y += manager.find(get_tex_for_type(button_corner_type::B))->tex.get_size().y;
	return l;
}