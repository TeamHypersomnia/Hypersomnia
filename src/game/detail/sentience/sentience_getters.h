#pragma once
#include <cstddef>
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
	return ::find_active_pe_absorption(self).has_value();
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
	if (self.dead()) {
		return false;
	}

	if (const auto sentience = self.template find<components::sentience>()) {
		return !sentience->is_dead();
	}

	return false;
}

template <class E>
bool sentient_and_conscious(const E& self) {
	if (self.dead()) {
		return false;
	}

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

/*
	Health together with the health that the active shield (armor) can still absorb with Personal Electricity.
	shield_mult converts Personal Electricity into health, 0 without an active shield.

	Measured in the maximums of health alone, so e.g. a full health with a full shield
	of the same absorbable health is 2.
*/

struct health_with_shield {
	real32 value = 0.f;
	real32 maximum = 0.f;
	real32 health_maximum = 0.f;
	real32 shield_mult = 0.f;

	real32 in_health_maximums(const real32 health) const {
		return health_maximum > 0.f ? health / health_maximum : 0.f;
	}

	real32 calc_current(const real32 added_value = 0.f) const {
		return in_health_maximums(value + added_value);
	}

	real32 calc_maximum() const {
		return in_health_maximums(maximum);
	}
};

template <class E>
inline health_with_shield get_health_with_shield(const E& character) {
	auto result = health_with_shield();

	if (const auto sentience = character.template find<components::sentience>()) {
		const auto& health = std::get<health_meter_instance>(sentience->meters);

		result.value = std::max(0.f, health.value);
		result.maximum = health.maximum;
		result.health_maximum = health.maximum;

		if (const auto absorption = ::find_active_pe_absorption(character)) {
			const auto& pe = std::get<personal_electricity_meter_instance>(sentience->meters);

			result.shield_mult = absorption->first.get_hp_mult();
			result.value += std::max(0.f, pe.value) * result.shield_mult;
			result.maximum += pe.maximum * result.shield_mult;
		}
	}

	return result;
}
