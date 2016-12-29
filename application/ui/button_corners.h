#pragma once
#include <array>
#include "augs/math/rects.h"
#include "game/assets/texture_id.h"
#include "game/resources/manager.h"

#include "augs/texture_baker/texture_with_image.h"

template <class L>
void for_each_button_corner(const ltrb rc, L callback, const bool flip = false) {
	auto& manager = get_resource_manager();

	std::array<assets::texture_id, 8> textures = {
		assets::texture_id::HOTBAR_BUTTON_LT,
		assets::texture_id::HOTBAR_BUTTON_RT,
		assets::texture_id::HOTBAR_BUTTON_RB,
		assets::texture_id::HOTBAR_BUTTON_LB,
		
		assets::texture_id::HOTBAR_BUTTON_L,
		assets::texture_id::HOTBAR_BUTTON_T,
		assets::texture_id::HOTBAR_BUTTON_R,
		assets::texture_id::HOTBAR_BUTTON_B,
	};

	if (flip) {
		std::swap(textures[0], textures[1]);
		std::swap(textures[2], textures[3]);

		std::swap(textures[4], textures[6]);
	}

	for(size_t i = 0; i < textures.size(); ++i) {
		const auto tex_id = textures[i];
		const auto& tex = manager.find(tex_id)->tex;
		const vec2 s = tex.get_size();

		ltrb target_rect;
		switch (i) {
		case 0:
			target_rect.set_size(s);
			target_rect.set_position(rc.left_top() - s);
			break;
		case 1:
			target_rect.set_size(s);
			target_rect.set_position(rc.right_top() - vec2(0, s.y));
			break;
		case 2:
			target_rect.set_size(s);
			target_rect.set_position(rc.right_bottom());
			break;
		case 3:
			target_rect.set_size(s);
			target_rect.set_position(rc.left_bottom() - vec2(s.x, 0));
			break;

		case 4:
			target_rect.set_size({ s.x, rc.h() });
			target_rect.set_position(rc.left_top() - vec2(s.x, 0));
			break;
		case 5:
			target_rect.set_size({ rc.w(), s.y });
			target_rect.set_position(rc.left_top() - vec2(0, s.y));
			break;
		case 6:
			target_rect.set_size({ s.x, rc.h() });
			target_rect.set_position(rc.right_top());
			break;
		case 7:
			target_rect.set_size({ rc.w(), s.y });
			target_rect.set_position(rc.left_bottom());
			break;
		}

		callback(tex_id, target_rect);
	}
}