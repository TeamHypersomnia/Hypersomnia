#include "../../systems/gui_system.h"
#include "../../globals/inventory.h"

rects::xywh<float> gui_system::get_rectangle_for_slot_function(slot_function f) {
	switch (f) {
	case slot_function::PRIMARY_HAND: return rects::xywh<float>(size.x - 100, size.y - 100, 33, 33);
	case slot_function::SHOULDER_SLOT: return rects::xywh<float>(size.x - 100, size.y - 200, 33, 33);
	case slot_function::SECONDARY_HAND: return rects::xywh<float>(size.x - 300, size.y - 100, 33, 33);
	case slot_function::TORSO_ARMOR_SLOT: return rects::xywh<float>(size.x - 200, size.y - 100, 33, 33);

	case slot_function::ITEM_DEPOSIT: return rects::xywh<float>(0, -100, 33, 33);
	default: assert(0);
	}
	assert(0);

	return rects::xywh<float>(0, 0, 0, 0);
}
