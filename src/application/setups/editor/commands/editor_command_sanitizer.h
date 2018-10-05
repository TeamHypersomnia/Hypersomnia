#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/commands/editor_command_traits.h"

template <class T, class F>
void sanitize_affected_entities(
	const editor_command_input& in,
	T& affected_entities,
	F&& id_getter
) {
	auto& cosm = in.get_cosmos();

	auto dead_eraser = [&](const auto& entry) {
		return cosm[id_getter(entry)].dead();
	};

	erase_if(affected_entities, dead_eraser);
}

template <class T>
void sanitize_affected_entities(
	const editor_command_input& in,
	T& affected_entities
) {
	auto& cosm = in.get_cosmos();

	auto dead_eraser = [&](const auto& entry) {
		return cosm[entry].dead();
	};

	erase_if(affected_entities, dead_eraser);
}

