#include "gui_system.h"
#include "augs/graphics/renderer.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"

#include "game/messages/intent_message.h"

#include "game/components/item_component.h"

#include "game/systems_stateless/crosshair_system.h"
#include "game/transcendental/entity_handle.h"

#include "game/transcendental/step.h"
#include "game/components/gui_element_component.h"

#include "augs/gui/rect_world.h"
#include "game/detail/gui/immediate_hud.h"
#include "game/enums/slot_function.h"

void gui_system::switch_to_gui_mode_and_back(fixed_step& step) {
	auto& intents = step.messages.get_queue<messages::intent_message>();
	auto& cosmos = step.cosm;

	for (auto& i : intents) {
		auto subject = cosmos[i.subject];

		if (subject.has<components::gui_element>()) {
			auto& gui = subject.get<components::gui_element>();

			if (i.intent == intent_type::SWITCH_TO_GUI && i.pressed_flag) {
				gui.is_gui_look_enabled = !gui.is_gui_look_enabled;
			}

			if (i.intent == intent_type::START_PICKING_UP_ITEMS) {
				//preview_due_to_item_picking_request = i.pressed_flag;
			}
		}

	}
}

void gui_system::advance_gui_elements(fixed_step& step) {
	auto& cosmos = step.cosm;

	for (auto& root : cosmos.get(processing_subjects::WITH_GUI_ELEMENT)) {
		root.get<components::gui_element>().advance(step);
	}
}