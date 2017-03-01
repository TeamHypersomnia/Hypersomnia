#pragma once
#include <array>
#include "augs/math/rects.h"
#include "game/assets/texture_id.h"
#include "game/resources/manager.h"

#include "augs/texture_atlas/texture_with_image.h"

enum class button_corner_type {
	LT,
	RT,
	RB,
	LB,

	L,
	T,
	R,
	B,
	
	LB_COMPLEMENT,

	LT_BORDER,
	RT_BORDER,
	RB_BORDER,
	LB_BORDER,

	L_BORDER,
	T_BORDER,
	R_BORDER,
	B_BORDER,
	
	LB_COMPLEMENT_BORDER,

	LT_INTERNAL_BORDER,
	RT_INTERNAL_BORDER,
	RB_INTERNAL_BORDER,
	LB_INTERNAL_BORDER,

	COUNT
};

bool is_button_border(const button_corner_type);
bool is_button_outside_border(const button_corner_type);
bool is_button_corner(const button_corner_type);
bool is_button_side(const button_corner_type);

struct button_corners_info {
	assets::texture_id lt_texture = assets::texture_id::INVALID;
	bool flip_horizontally = true;

	assets::texture_id get_tex_for_type(button_corner_type) const;

	ltrb cornered_rc_to_internal_rc(ltrb) const;
	ltrb internal_rc_to_cornered_rc(ltrb) const;
	vec2i cornered_size_to_internal_size(vec2i) const;
	vec2i internal_size_to_cornered_size(vec2i) const;

	template <class L>
	void for_each_button_corner(const ltrb rc, L callback) const {
		auto& manager = get_resource_manager();

		for (auto i = button_corner_type::LT; i < button_corner_type::COUNT; i = static_cast<button_corner_type>(static_cast<int>(i) + 1)) {
			const auto tex_id = get_tex_for_type(i);
			const auto* const found_tex = manager.find(tex_id);

			if (found_tex == nullptr) {
				continue;
			}

			const auto& tex = found_tex->tex;
			const vec2 s = tex.get_size();

			ltrb target_rect;
			
			if (i == button_corner_type::LT || i == button_corner_type::LT_BORDER || i == button_corner_type::LT_INTERNAL_BORDER) {
				target_rect.set_size(s);
				target_rect.set_position(rc.left_top() - s);
			}
			
			else if (i == button_corner_type::RT || i == button_corner_type::RT_BORDER || i == button_corner_type::RT_INTERNAL_BORDER) {
				target_rect.set_size(s);
				target_rect.set_position(rc.right_top() - vec2(0, s.y));
			}

			else if (i == button_corner_type::RB || i == button_corner_type::RB_BORDER || i == button_corner_type::RB_INTERNAL_BORDER) {
				target_rect.set_size(s);
				target_rect.set_position(rc.right_bottom());
			}
			
			else if (i == button_corner_type::LB || i == button_corner_type::LB_BORDER || i == button_corner_type::LB_INTERNAL_BORDER) {
				target_rect.set_size(s);
				target_rect.set_position(rc.left_bottom() - vec2(s.x, 0));
			}


			else if (i == button_corner_type::L || i == button_corner_type::L_BORDER) {
				target_rect.set_size({ s.x, rc.h() });
				target_rect.set_position(rc.left_top() - vec2(s.x, 0));
			}

			else if (i == button_corner_type::T || i == button_corner_type::T_BORDER) {
				target_rect.set_size({ rc.w(), s.y });
				target_rect.set_position(rc.left_top() - vec2(0, s.y));
			}

			else if (i == button_corner_type::R || i == button_corner_type::R_BORDER) {
				target_rect.set_size({ s.x, rc.h() });
				target_rect.set_position(rc.right_top());
			}

			else if (i == button_corner_type::B || i == button_corner_type::B_BORDER) {
				target_rect.set_size({ rc.w(), s.y });
				target_rect.set_position(rc.left_bottom());
			}


			else if (i == button_corner_type::LB_COMPLEMENT || i == button_corner_type::LB_COMPLEMENT_BORDER) {
				target_rect.set_size(s);

				if (flip_horizontally) {
					target_rect.set_position(rc.right_bottom());
				}
				else {
					target_rect.set_position(rc.left_bottom() - vec2(s.x, 0));
				}
			}
			else {
				ensure("Unsupported button border type" && false);
			}

			callback(i, tex_id, target_rect);
		}
	}
};