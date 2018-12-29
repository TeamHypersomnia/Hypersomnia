#pragma once
#include "game/components/sentience_component.h"

template <class E>
bool sentient_and_unconscious(const E& self) {
	if (const auto sentience = self.template find<components::sentience>()) {
		return !sentience->is_conscious();
	}

	return false;
}

template <class E>
bool electric_shield_enabled(const E& self) {
	if (const auto sentience = self.template find<components::sentience>()) {
		return sentience->template get<electric_shield_perk_instance>().timing.is_enabled(self.get_cosmos().get_clock());
	}

	return false;
}

template <class E>
bool sentient_and_vulnerable(const E& self) {
	if (const auto sentience = self.template find<components::sentience>()) {
		return !sentience->is_dead() && !electric_shield_enabled(self);
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
