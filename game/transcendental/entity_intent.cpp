#include "entity_intent.h"
#include "game/global/input_context.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/components/gui_element_component.h"

bool operator==(const std::vector<entity_intent>& a, const std::vector<entity_intent>& b) {
	return !(a != b);
}

bool operator!=(const std::vector<entity_intent>& a, const std::vector<entity_intent>& b) {
	if (a.size() != b.size()) {
		return true;
	}

	if (std::memcmp(a.data(), b.data(), sizeof(entity_intent) * a.size())) {
		return true;
	}

	return false;
}

bool entity_intent::uses_mouse_motion() const {
	return
		intent == intent_type::MOVE_CROSSHAIR ||
		intent == intent_type::CROSSHAIR_PRIMARY_ACTION ||
		intent == intent_type::CROSSHAIR_SECONDARY_ACTION;
}

bool entity_intent::operator!=(const entity_intent& b) const {
	return std::make_tuple(intent, mouse_rel, pressed_flag) != std::make_tuple(b.intent, b.mouse_rel, b.pressed_flag);
}

bool entity_intent::from_raw_state(const input_context& context, const augs::window::event::change& raw) {
	bool found_context_entry = false;

	if (raw.was_key_pressed() || raw.was_key_released()) {
		pressed_flag = raw.was_key_pressed();

		const auto found_intent = context.key_to_intent.find(raw.key);
		if (found_intent != context.key_to_intent.end()) {
			intent = (*found_intent).second;
			found_context_entry = true;
		}
	}
	else {
		pressed_flag = true;

		const auto found_intent = context.event_to_intent.find(raw.msg);
		if (found_intent != context.event_to_intent.end()) {
			intent = (*found_intent).second;
			found_context_entry = true;
		}
	}

	mouse_rel = raw.mouse.rel;
	return found_context_entry;
}

bool entity_intent::from_raw_state_and_possible_gui_receiver(const input_context& context, const augs::window::event::change& raw, const const_entity_handle& gui_receiver) {
	bool should_gui_fetch_this = false;

	if (gui_receiver.alive()) {
		if (gui_receiver.has<components::gui_element>()) {
			if (
				raw.msg != augs::window::event::message::keydown &&
				raw.msg != augs::window::event::message::keyup &&
				gui_receiver.get<components::gui_element>().is_gui_look_enabled) {
				should_gui_fetch_this = true;
			}
		}
	}

	if (should_gui_fetch_this) {
		state_for_gui = raw;
		has_state_for_gui = true;

		return true;
	}
	else {
		return from_raw_state(context, raw);
	}
}