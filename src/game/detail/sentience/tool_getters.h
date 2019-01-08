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
