#pragma once
#include "game/detail/sentience/pe_absorption.h"

template <class E>
std::optional<std::pair<pe_absorption_info, entity_id>> find_active_pe_absorption(const E& subject) {
	if (const auto armor_slot = subject[slot_function::TORSO_ARMOR]) {
		if (const auto tool_item = armor_slot.get_item_if_any()) {
			if (const auto tool = tool_item.template find<invariants::tool>()) {
				const auto& abso = tool->pe_absorption;

				if (abso.is_set()) {
					return std::make_pair(abso, tool_item.get_id());
				}
			}
		}
	}

	return std::nullopt;
}

template <class E>
real32 get_movement_speed_mult(const E& subject) {
	real32 freed_pe_ratio = 0.f;

	if (find_active_pe_absorption(subject)) {
		const auto& sentience = subject.template get<components::sentience>();
		const auto& personal_electricity = sentience.template get<personal_electricity_meter_instance>();
		freed_pe_ratio = 1.f - personal_electricity.get_ratio();
	}

	if (const auto armor_slot = subject[slot_function::TORSO_ARMOR]) {
		if (const auto tool_item = armor_slot.get_item_if_any()) {
			if (const auto tool = tool_item.template find<invariants::tool>()) {
				auto mult = tool->movement_speed_mult;

				if (mult < 1.f) {
					mult += (1.f - mult) * freed_pe_ratio;
				}

				return mult;
			}
		}
	}

	return 1.f;
}
