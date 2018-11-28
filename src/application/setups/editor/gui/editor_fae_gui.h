#pragma once
#include <vector>
#include <unordered_map>
#include "3rdparty/imgui/imgui.h"

#include "application/setups/editor/commands/editor_command_structs.h"
#include "application/setups/editor/property_editor/fae/fae_tree_structs.h"

#include "augs/misc/imgui/standard_window_mixin.h"

namespace augs {
	struct introspection_access;
}

struct editor_settings;
struct editor_command_input;

struct editor_fae_gui_input {
	const property_editor_settings& settings;
	const editor_command_input command_in;
};

using fae_selections_type = std::unordered_set<entity_id>;

template <class E>
using make_flavour_to_entities_map = std::unordered_map<typed_entity_flavour_id<E>, std::vector<typed_entity_id<E>>>;
using flavour_to_entities_type = per_entity_type_container<make_flavour_to_entities_map>;

/*
	"fae tree" is a shorthand for "flavours and entities tree".
*/

struct editor_fae_gui_base : standard_window_mixin<editor_fae_gui_base> {
	using base = standard_window_mixin<editor_fae_gui_base>;
	using base::base;
	using introspect_base = base;

	template <class T>
	using make_ticked_flavours = std::unordered_set<typed_entity_flavour_id<T>>;

	template <class T>
	using make_ticked_entities = std::unordered_set<typed_entity_id<T>>;

	using ticked_flavours_type = per_entity_type_container<make_ticked_flavours>;
	using ticked_entities_type = per_entity_type_container<make_ticked_entities>;

	void interrupt_tweakers();

	auto get_hovered_guid() const {
		return entities_tree_data.hovered_guid;
	}

	void do_view_mode_switch();

	void clear_dead_entities(const cosmos&);

protected:
	friend augs::introspection_access;
	entities_tree_state entities_tree_data;
	ImGuiTextFilter filter;

	property_editor_state property_editor_data;
	fae_view_type view_mode = fae_view_type::FLAVOURS;

	flavour_to_entities_type cached_flavour_to_entities;

	fae_tree_input make_fae_input(editor_fae_gui_input, bool);
};

struct fae_tree_output {
	fae_tree_filter filter;
	std::optional<entity_flavour_id> instantiate_id;
};

struct editor_fae_gui : editor_fae_gui_base {
	using base = editor_fae_gui_base;
	using base::base;
	using introspect_base = base;

	fae_tree_output perform(
		editor_fae_gui_input, 
		fae_selections_type& all_selections
	);

	void clear_dead_entities(const cosmos&);

private:
	ticked_flavours_type ticked_flavours;
	ticked_entities_type cached_ticked_entities;
};

struct editor_selected_fae_gui : editor_fae_gui_base {
	using base = editor_fae_gui_base;
	using base::base;
	using introspect_base = base;

	fae_tree_output perform(
		editor_fae_gui_input,
		const fae_selections_type& only_match_entities
	);

	void clear_dead_entities(const cosmos&);

private:
	ticked_flavours_type ticked_flavours;
	ticked_entities_type ticked_entities;
};
