#pragma once
#include <array>
#include "augs/math/rects.h"
#include "game/assets/texture_id.h"
#include "game/resources/manager.h"

#include "augs/texture_baker/texture_with_image.h"

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
	COUNT
};

bool is_button_corner(const button_corner_type);
bool is_button_side(const button_corner_type);

struct button_corners_info {
	assets::texture_id lt_texture = assets::texture_id::HOTBAR_BUTTON_LT;
	bool flip_horizontally = true;

	assets::texture_id get_tex_for_type(button_corner_type) const;

	ltrb cornered_rc_to_internal_rc(ltrb) const;
	ltrb internal_rc_to_cornered_rc(ltrb) const;
	vec2i cornered_size_to_internal_size(vec2i) const;
	vec2i internal_size_to_cornered_size(vec2i) const;

	template <class L>
	void for_each_button_corner(const ltrb rc, L callback) const {
		auto& manager = get_resource_manager();

		for (auto i = button_corner_type::LT; i < button_corner_type::COUNT; i = button_corner_type(static_cast<int>(i) + 1)) {
			const auto tex_id = get_tex_for_type(i);
			const auto& tex = manager.find(tex_id)->tex;
			const vec2 s = tex.get_size();

			ltrb target_rect;
			switch (i) {
			case button_corner_type::LT:
				target_rect.set_size(s);
				target_rect.set_position(rc.left_top() - s);
				LOG_NVPS(s);
				break;
			case button_corner_type::RT:
				target_rect.set_size(s);
				target_rect.set_position(rc.right_top() - vec2(0, s.y));
				break;
			case button_corner_type::RB:
				target_rect.set_size(s);
				target_rect.set_position(rc.right_bottom());
				break;
			case button_corner_type::LB:
				target_rect.set_size(s);
				target_rect.set_position(rc.left_bottom() - vec2(s.x, 0));
				break;

			case button_corner_type::L:
				target_rect.set_size({ s.x, rc.h() });
				target_rect.set_position(rc.left_top() - vec2(s.x, 0));
				break;
			case button_corner_type::T:
				target_rect.set_size({ rc.w(), s.y });
				target_rect.set_position(rc.left_top() - vec2(0, s.y));
				break;
			case button_corner_type::R:
				target_rect.set_size({ s.x, rc.h() });
				target_rect.set_position(rc.right_top());
				break;
			case button_corner_type::B:
				target_rect.set_size({ rc.w(), s.y });
				target_rect.set_position(rc.left_bottom());
				break;

			case button_corner_type::LB_COMPLEMENT:
				target_rect.set_size(s);

				if (flip_horizontally) {
					target_rect.set_position(rc.right_bottom());
				}
				else {
					target_rect.set_position(rc.left_bottom() - vec2(s.x, 0));
				}

				break;
			}

			callback(i, tex_id, target_rect);
		}
	}
};