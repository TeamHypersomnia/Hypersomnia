#pragma once
#include <optional>
#include "augs/math/vec2.h"

enum class intent_change : unsigned char {
	PRESSED,
	RELEASED
};

template <class K>
std::optional<intent_change> to_intent_change(const K& ch) {
	switch (ch) {
		case K::PRESSED: return intent_change::PRESSED;
		case K::RELEASED: return intent_change::RELEASED;
		default: return std::nullopt;
	}
}

template <class intent_type_enum>
struct basic_input_intent {
	intent_type_enum intent = intent_type_enum::INVALID;
	intent_change change = intent_change::PRESSED;

	bool was_pressed() const {
		return change == intent_change::PRESSED;
	}

	bool is_set() const {
		return intent != intent_type_enum::INVALID;
	}

	bool operator==(const basic_input_intent& b) const {
		return
			intent == b.intent
			&& change == b.change
		;
	}

	bool operator!=(const basic_input_intent& b) const {
		return !operator==(b);
	}
};