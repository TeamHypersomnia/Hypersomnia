#include "button_corners.h"

ltrb cornered_button_size_to_internal_size(ltrb l) {
	auto& manager = get_resource_manager();
	l.l += manager.find(assets::texture_id::HOTBAR_BUTTON_L)->tex.get_size().x;
	l.t += manager.find(assets::texture_id::HOTBAR_BUTTON_T)->tex.get_size().y;
	l.r -= manager.find(assets::texture_id::HOTBAR_BUTTON_R)->tex.get_size().x;
	l.b -= manager.find(assets::texture_id::HOTBAR_BUTTON_B)->tex.get_size().y;
	return l;
}

ltrb internal_size_to_cornered_button_size(ltrb l) {
	auto& manager = get_resource_manager();
	l.l -= manager.find(assets::texture_id::HOTBAR_BUTTON_L)->tex.get_size().x;
	l.t -= manager.find(assets::texture_id::HOTBAR_BUTTON_T)->tex.get_size().y;
	l.r += manager.find(assets::texture_id::HOTBAR_BUTTON_R)->tex.get_size().x;
	l.b += manager.find(assets::texture_id::HOTBAR_BUTTON_B)->tex.get_size().y;
	return l;
}

vec2i cornered_button_size_to_internal_size(vec2i l) {
	auto& manager = get_resource_manager();
	l.x -= manager.find(assets::texture_id::HOTBAR_BUTTON_L)->tex.get_size().x;
	l.y -= manager.find(assets::texture_id::HOTBAR_BUTTON_T)->tex.get_size().y;
	l.x -= manager.find(assets::texture_id::HOTBAR_BUTTON_R)->tex.get_size().x;
	l.y -= manager.find(assets::texture_id::HOTBAR_BUTTON_B)->tex.get_size().y;
	return l;
}

vec2i internal_size_to_cornered_button_size(vec2i l) {
	auto& manager = get_resource_manager();
	l.x += manager.find(assets::texture_id::HOTBAR_BUTTON_L)->tex.get_size().x;
	l.y += manager.find(assets::texture_id::HOTBAR_BUTTON_T)->tex.get_size().y;
	l.x += manager.find(assets::texture_id::HOTBAR_BUTTON_R)->tex.get_size().x;
	l.y += manager.find(assets::texture_id::HOTBAR_BUTTON_B)->tex.get_size().y;
	return l;
}
