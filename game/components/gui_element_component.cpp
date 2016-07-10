#include "gui_element_component.h"

namespace components {
	rects::xywh<float> gui_element::get_rectangle_for_slot_function(slot_function f) const {
		switch (f) {
		case slot_function::PRIMARY_HAND: return rects::xywh<float>(100, 0, 33, 33);
		case slot_function::SHOULDER_SLOT: return rects::xywh<float>(100, -100, 33, 33);
		case slot_function::SECONDARY_HAND: return rects::xywh<float>(-100, 0, 33, 33);
		case slot_function::TORSO_ARMOR_SLOT: return rects::xywh<float>(0, 0, 33, 33);

		case slot_function::ITEM_DEPOSIT: return rects::xywh<float>(0, -100, 33, 33);

		case slot_function::GUN_DETACHABLE_MAGAZINE: return rects::xywh<float>(0, 50, 33, 33);
		case slot_function::GUN_CHAMBER: return rects::xywh<float>(0, -50, 33, 33);
		case slot_function::GUN_BARREL: return rects::xywh<float>(-50, 0, 33, 33);
		default: ensure(0);
		}
		ensure(0);

		return rects::xywh<float>(0, 0, 0, 0);
	}

	vec2i gui_element::get_initial_position_for_special_control(special_control s) const {
		switch (s) {
		case special_control::DROP_ITEM: return vec2i(size.x - 150, 30);
		}
	}

	vec2 gui_element::initial_inventory_root_position() const {
		return vec2(size.x - 250, size.y - 200);
	}

	void gui_element::draw_complete_gui_for_camera_rendering_request(viewing_step& step) const {

	}
}