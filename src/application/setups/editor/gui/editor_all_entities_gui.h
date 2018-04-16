#pragma once
#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/property_editor/fae_tree_structs.h"

#include "application/setups/editor/gui/standard_window_mixin.h"

struct editor_settings;
struct editor_command_input;

struct editor_all_entities_gui : standard_window_mixin<editor_all_entities_gui> {
	using base = standard_window_mixin<editor_all_entities_gui>;
	using base::base;

	fae_tree_filter perform(
		const editor_settings&,
		const std::unordered_set<entity_id>* only_match_entities,
	   	editor_command_input
	);

	void interrupt_tweakers();

	auto get_hovered_guid() const {
		return fae_tree_data.hovered_guid;
	}

private:
	fae_tree_state fae_tree_data;
	property_editor_state property_editor_data;
};
