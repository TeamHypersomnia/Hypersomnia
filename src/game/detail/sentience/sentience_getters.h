#pragma once
#include "game/components/sentience_component.h"
#include "game/detail/sentience/tool_getters.h"

template <class E>
bool sentient_and_unconscious(const E& self) {
	if (const auto sentience = self.template find<components::sentience>()) {
		return !sentience->is_conscious();
	}

	return false;
}


template <class E>
bool any_shield_active(const E& self) {
	return ::find_active_pe_absorption(self) != std::nullopt;
}

template <class E>
bool sentient_and_vulnerable(const E& self) {
	if (const auto sentience = self.template find<components::sentience>()) {
		return !sentience->is_dead() && !any_shield_active(self);
	}

	return false;
}

template <class E>
bool sentient_and_alive(const E& self) {
	if (const auto sentience = self.template find<components::sentience>()) {
		return !sentience->is_dead();
	}

	return false;
}

template <class E>
bool sentient_and_conscious(const E& self) {
	if (const auto sentience = self.template find<components::sentience>()) {
		return sentience->is_conscious();
	}

	return false;
}

template <class E>
bool get_hand_flag(const E& self, const std::size_t index) {
	if (const auto sentience = self.template find<components::sentience>()) {
		const auto& now = self.get_cosmos().get_timestamp();

		return 
			sentience->hand_flags[index] 
			|| sentience->when_hand_pressed[index] == now
		;
	}

	return false;
}

template <class E>
inline real32 get_shield_ratio(const E& character) {
	if (!any_shield_active(character)) {
		return 0.0f;
	}

	if (const auto sentience = character.template find<components::sentience>()) {
		return std::get<personal_electricity_meter_instance>(sentience->meters).get_ratio();
	}

	return 0.0f;
}

template <class E>
inline real32 get_health_ratio(const E& character) {
	if (const auto sentience = character.template find<components::sentience>()) {
		return std::get<health_meter_instance>(sentience->meters).get_ratio();
	}

	return 0.0f;
}
