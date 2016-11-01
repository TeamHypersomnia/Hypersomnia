#pragma once
#include "game/enums/intent_type.h"
#include "augs/window_framework/event.h"
#include "augs/padding_byte.h"

#include "game/transcendental/entity_handle_declaration.h"

struct input_context;

struct key_and_mouse_intent {
	intent_type intent = intent_type::NONE;
	vec2t<short> mouse_rel;
	bool pressed_flag = false;

	bool from_raw_state(const input_context&, const augs::window::event::change&);
	bool uses_mouse_motion() const;

	bool operator==(const key_and_mouse_intent& b) const;
	bool operator!=(const key_and_mouse_intent& b) const;
};

struct entity_intent : key_and_mouse_intent {
	augs::window::event::change event_for_gui;
	bool has_event_for_gui = false;

	bool from_raw_state_and_possible_gui_receiver(const input_context&, const augs::window::event::change&, const const_entity_handle& gui_receiver);

	bool operator==(const entity_intent& b) const;
	bool operator!=(const entity_intent& b) const;
};

bool operator==(const std::vector<entity_intent>& a, const std::vector<entity_intent>& b);
bool operator!=(const std::vector<entity_intent>& a, const std::vector<entity_intent>& b);

namespace augs {
	template<class A>
	bool read_object(A& ar, entity_intent& intent) {
		if (!read_object(ar, intent.intent)) return false;
		if (!read_object(ar, intent.pressed_flag)) return false;
		if (!read_object(ar, intent.has_event_for_gui)) return false;

		if (intent.uses_mouse_motion()) {
			if (!read_object(ar, intent.mouse_rel)) {
				return false;
			}
		}

		if (intent.has_event_for_gui) {
			if (!read_object(ar, intent.event_for_gui)) {
				return false;
			}
		}

		return true;
	}

	template<class A>
	void write_object(A& ar, const entity_intent& intent) {
		write_object(ar, intent.intent);
		write_object(ar, intent.pressed_flag);
		write_object(ar, intent.has_event_for_gui);

		if (intent.uses_mouse_motion()) {
			write_object(ar, intent.mouse_rel);
		}

		if (intent.has_event_for_gui) {
			write_object(ar, intent.event_for_gui);
		}
	}
}