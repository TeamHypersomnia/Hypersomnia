#pragma once
#include "augs/math/vec2.h"

template <class intent_type_enum>
struct basic_game_intent {
	intent_type_enum intent = intent_type_enum::INVALID;
	bool is_pressed = false;

	bool is_set() const {
		return intent != intent_type_enum::INVALID;
	}

	bool operator==(const basic_game_intent& b) const {
		return
			intent == b.intent
			&& is_pressed == b.is_pressed
		;
	}

	bool operator!=(const basic_game_intent& b) const {
		return !operator==(b);
	}
};