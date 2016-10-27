#include "gui_element_component.h"
#include "game/transcendental/step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/inventory_slot_handle.h"
#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/item_component.h"

namespace components {
	gui_element::gui_element() :
		drop_item_icon(augs::gui::material(assets::texture_id::DROP_HAND_ICON, red))
	{
	}

	rects::xywh<float> gui_element::get_rectangle_for_slot_function(const slot_function f) const {
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

	vec2i gui_element::get_initial_position_for_special_control(const special_control s) const {
		switch (s) {
		case special_control::DROP_ITEM: return vec2i(size.x - 150, 30);
		}
	}

	vec2 gui_element::initial_inventory_root_position() const {
		return vec2(size.x - 250, size.y - 200);
	}

	void gui_element::draw_complete_gui_for_camera_rendering_request(const const_entity_handle& handle, viewing_step& step) {
		//r.renderer.push_triangles(gui.draw_triangles());
		//
		//if (is_gui_look_enabled)
		//	gui.draw_cursor_and_tooltip(r);
	}
}