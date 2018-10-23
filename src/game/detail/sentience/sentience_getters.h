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
bool sentient_and_not_dead(const E& self) {
	if (const auto sentience = self.template find<components::sentience>()) {
		return !sentience->is_dead();
	}

	return false;
}
