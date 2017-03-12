#include "key_and_mouse_intent.h"
#include "augs/misc/input_context.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/detail/gui/character_gui.h"

bool operator==(const key_and_mouse_intent_vector& a, const key_and_mouse_intent_vector& b) {
	return compare_containers(a, b);
}

bool operator!=(const key_and_mouse_intent_vector& a, const key_and_mouse_intent_vector& b) {
	return !(a == b);
}

bool key_and_mouse_intent::uses_mouse_motion() const {
	return
		intent == intent_type::MOVE_CROSSHAIR ||
		intent == intent_type::CROSSHAIR_PRIMARY_ACTION ||
		intent == intent_type::CROSSHAIR_SECONDARY_ACTION;
}

bool key_and_mouse_intent::operator==(const key_and_mouse_intent& b) const {
	return std::make_tuple(intent, mouse_rel, is_pressed) == std::make_tuple(b.intent, b.mouse_rel, b.is_pressed);
}

bool key_and_mouse_intent::operator!=(const key_and_mouse_intent& b) const {
	return !operator==(b);
}