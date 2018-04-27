#pragma once
#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/property_editor/fae_tree_structs.h"

#include "application/setups/editor/gui/standard_window_mixin.h"

struct editor_settings;
struct editor_command_input;
class loaded_image_caches_map;

struct editor_fae_gui_input {
	const property_editor_settings& settings;
	const std::unordered_set<entity_id>* const only_match_entities;
	const editor_command_input command_in;
	const loaded_image_caches_map& image_caches;
};

struct editor_fae_gui : standard_window_mixin<editor_fae_gui> {
	using base = standard_window_mixin<editor_fae_gui>;
	using base::base;
	using introspect_base = base;

	fae_tree_filter perform(editor_fae_gui_input);

	void interrupt_tweakers();

	auto get_hovered_guid() const {
		return fae_tree_data.hovered_guid;
	}

private:
	fae_tree_state fae_tree_data;
	property_editor_state property_editor_data;
};
