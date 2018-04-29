#pragma once
#include <vector>
#include <unordered_map>

#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/property_editor/fae_tree_structs.h"

#include "application/setups/editor/gui/standard_window_mixin.h"

namespace augs {
	struct introspection_access;
}

struct editor_settings;
struct editor_command_input;
class loaded_image_caches_map;

struct editor_fae_gui_input {
	const property_editor_settings& settings;
	const editor_command_input command_in;
	const loaded_image_caches_map& image_caches;
};

struct editor_fae_gui_base : standard_window_mixin<editor_fae_gui_base> {
	using base = standard_window_mixin<editor_fae_gui_base>;
	using base::base;
	using introspect_base = base;

	void interrupt_tweakers();

	auto get_hovered_guid() const {
		return fae_tree_data.hovered_guid;
	}

protected:
	friend augs::introspection_access;
	// GEN INTROSPECTOR struct editor_fae_gui
	fae_tree_state fae_tree_data;
	// END GEN INTROSPECTOR
	property_editor_state property_editor_data;

	fae_tree_input make_fae_input(editor_fae_gui_input, bool);
};

struct editor_fae_gui : editor_fae_gui_base {
	using base = editor_fae_gui_base;
	using base::base;
	using introspect_base = base;

	fae_tree_filter perform(editor_fae_gui_input);
};

using fae_selections_type = std::unordered_set<entity_id>;

struct editor_selected_fae_gui : editor_fae_gui_base {
	template <class E>
	using make_flavour_to_entities_map = std::unordered_map<typed_entity_flavour_id<E>, std::vector<typed_entity_id<E>>>;
	using resolved_container_type = per_entity_type_container<make_flavour_to_entities_map>;

	using base = editor_fae_gui_base;
	using base::base;
	using introspect_base = base;

	fae_tree_filter perform(
		editor_fae_gui_input,
		const fae_selections_type& only_match_entities
	);

private:
	resolved_container_type per_native_type;
};
