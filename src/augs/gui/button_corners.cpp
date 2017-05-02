#include "button_corners.h"

using namespace assets;

std::string get_filename_for(const button_corner_type t) {
	static const std::array<std::string, static_cast<size_t>(button_corner_type::COUNT)> stems = {
		"inside",
		"lt",
		"rt",
		"rb",
		"lb",
		"l",
		"t",
		"r",
		"b",
		"lb_complement",
		"lt_border",
		"rt_border",
		"rb_border",
		"lb_border",
		"l_border",
		"t_border",
		"r_border",
		"b_border",
		"lb_complement_border",
		"lt_internal_border",
		"rt_internal_border",
		"rb_internal_border",
		"lb_internal_border"
	};

	const auto idx = static_cast<unsigned>(t);

	ensure(idx < stems.size())

	return stems[idx];
}

bool is_button_corner(const button_corner_type t) {
	if (
		t == button_corner_type::LT
		|| t == button_corner_type::RT
		|| t == button_corner_type::RB
		|| t == button_corner_type::LB

		|| t == button_corner_type::LT_BORDER
		|| t == button_corner_type::RT_BORDER
		|| t == button_corner_type::RB_BORDER
		|| t == button_corner_type::LB_BORDER

		|| t == button_corner_type::LT_INTERNAL_BORDER
		|| t == button_corner_type::RT_INTERNAL_BORDER
		|| t == button_corner_type::RB_INTERNAL_BORDER
		|| t == button_corner_type::LB_INTERNAL_BORDER
	) {
		return true;
	}

	return false;
}

bool is_button_outside_border(const button_corner_type t) {
	if (
		t == button_corner_type::L_BORDER
		|| t == button_corner_type::T_BORDER
		|| t == button_corner_type::R_BORDER
		|| t == button_corner_type::B_BORDER

		|| t == button_corner_type::LT_BORDER
		|| t == button_corner_type::RT_BORDER
		|| t == button_corner_type::RB_BORDER
		|| t == button_corner_type::LB_BORDER

		|| t == button_corner_type::LB_COMPLEMENT_BORDER
		) {
		return true;
	}

	return false;
}

bool is_lb_complement(const button_corner_type t) {
	if (
		t == button_corner_type::LB_COMPLEMENT
		|| t == button_corner_type::LB_COMPLEMENT_BORDER
	) {
		return true;
	}

	return false;
}

bool is_button_border(const button_corner_type t) {
	if (
		is_button_outside_border(t)
		|| t == button_corner_type::LT_INTERNAL_BORDER
		|| t == button_corner_type::RT_INTERNAL_BORDER
		|| t == button_corner_type::RB_INTERNAL_BORDER
		|| t == button_corner_type::LB_INTERNAL_BORDER

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

		|| t == button_corner_type::L_BORDER
		|| t == button_corner_type::T_BORDER
		|| t == button_corner_type::R_BORDER
		|| t == button_corner_type::B_BORDER
		) {
		return true;
	}

	return false;
}

game_image_id button_corners_info::get_tex_for_type(button_corner_type t) const {
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
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 0);
		break;

	case button_corner_type::LT:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 1);
		break;
	case button_corner_type::RT:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 2);
		break;
	case button_corner_type::RB:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 3);
		break;
	case button_corner_type::LB:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 4);
		break;

	case button_corner_type::L:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 5);
		break;
	case button_corner_type::T:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 6);
		break;
	case button_corner_type::R:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 7);
		break;
	case button_corner_type::B:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 8);
		break;

	case button_corner_type::LB_COMPLEMENT:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 9);
		break;

	case button_corner_type::LT_BORDER:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 10);
		break;
	case button_corner_type::RT_BORDER:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 11);
		break;
	case button_corner_type::RB_BORDER:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 12);
		break;
	case button_corner_type::LB_BORDER:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 13);
		break;


	case button_corner_type::L_BORDER:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 14);
		break;
	case button_corner_type::T_BORDER:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 15);
		break;
	case button_corner_type::R_BORDER:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 16);
		break;
	case button_corner_type::B_BORDER:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 17);
		break;

	case button_corner_type::LB_COMPLEMENT_BORDER:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 18);
		break;

	case button_corner_type::LT_INTERNAL_BORDER:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 19);
		break;
	case button_corner_type::RT_INTERNAL_BORDER:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 20);
		break;
	case button_corner_type::RB_INTERNAL_BORDER:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 21);
		break;
	case button_corner_type::LB_INTERNAL_BORDER:
		return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 22);
		break;

	default: ensure(false); return static_cast<game_image_id>(static_cast<unsigned>(inside_texture) + 0); break;
	}
}

ltrb button_corners_info::cornered_rc_to_internal_rc(ltrb l) const {
	const auto& manager = get_assets_manager();
	l.l += manager[get_tex_for_type(button_corner_type::L)].get_size().x;
	l.t += manager[get_tex_for_type(button_corner_type::T)].get_size().y;
	l.r -= manager[get_tex_for_type(button_corner_type::R)].get_size().x;
	l.b -= manager[get_tex_for_type(button_corner_type::B)].get_size().y;
	return l;
}

ltrb button_corners_info::internal_rc_to_cornered_rc(ltrb l) const {
	const auto& manager = get_assets_manager();
	l.l += manager[get_tex_for_type(button_corner_type::L)].get_size().x;
	l.t += manager[get_tex_for_type(button_corner_type::T)].get_size().y;
	l.r -= manager[get_tex_for_type(button_corner_type::R)].get_size().x;
	l.b -= manager[get_tex_for_type(button_corner_type::B)].get_size().y;
	return l;
}

vec2i button_corners_info::cornered_size_to_internal_size(vec2i l) const {
	const auto& manager = get_assets_manager();
	l.x -= manager[get_tex_for_type(button_corner_type::L)].get_size().x;
	l.x -= manager[get_tex_for_type(button_corner_type::R)].get_size().x;
	l.y -= manager[get_tex_for_type(button_corner_type::T)].get_size().y;
	l.y -= manager[get_tex_for_type(button_corner_type::B)].get_size().y;
	return l;
}

vec2i button_corners_info::internal_size_to_cornered_size(vec2i l) const {
	const auto& manager = get_assets_manager();
	l.x += manager[get_tex_for_type(button_corner_type::L)].get_size().x;
	l.x += manager[get_tex_for_type(button_corner_type::R)].get_size().x;
	l.y += manager[get_tex_for_type(button_corner_type::T)].get_size().y;
	l.y += manager[get_tex_for_type(button_corner_type::B)].get_size().y;
	return l;
}