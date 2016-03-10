#include "../../systems/gui_system.h"
#include "../../globals/inventory.h"

rects::xywh<float> gui_system::get_rectangle_for_slot_function(slot_function f) {
	switch (f) {
	case slot_function::PRIMARY_HAND: return rects::xywh<float>(100, 0, 33, 33);
	case slot_function::SHOULDER_SLOT: return rects::xywh<float>(100, -100, 33, 33);
	case slot_function::SECONDARY_HAND: return rects::xywh<float>(-100, 0, 33, 33);
	case slot_function::TORSO_ARMOR_SLOT: return rects::xywh<float>(0, 0, 33, 33);

	case slot_function::ITEM_DEPOSIT: return rects::xywh<float>(0, -100, 33, 33);

	case slot_function::GUN_DETACHABLE_MAGAZINE: return rects::xywh<float>(0, 50, 33, 33);
	case slot_function::GUN_CHAMBER: return rects::xywh<float>(0, -50, 33, 33);
	default: assert(0);
	}
	assert(0);

	return rects::xywh<float>(0, 0, 0, 0);
}

vec2 gui_system::initial_inventory_root_position() {
	return vec2(size.x - 250, size.y - 100);
}

