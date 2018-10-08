#pragma once
#include <utility>
#include "augs/templates/snapshotted_player_step_type.h"

struct editor_folder;
class editor_entity_selector;

namespace sol {
	class state;
}

class editor_entity_mover;
struct editor_fae_gui;

class cosmos;

struct entity_id;
struct all_viewables_defs;
struct all_logical_assets;
struct editor_history;
struct editor_settings;
class editor_player;

struct editor_command_input {
	sol::state& lua;
	const editor_settings& settings;
	editor_folder& folder;
	editor_entity_selector& selector;

	editor_fae_gui& fae_gui;
	editor_entity_mover& mover;

	cosmos& get_cosmos() const; 

	editor_history& get_history() const;
	editor_player& get_player() const;
	all_viewables_defs& get_viewable_defs() const;
	const all_logical_assets& get_logical_assets() const;

	augs::snapshotted_player_step_type get_current_step() const;

	void purge_selections() const;
	void interrupt_tweakers() const;
	void clear_selection_of(entity_id) const;
};

template <class E, class T>
const T& post_editor_command(const E& in, T&& cmd) {
	return in.get_history().execute_new(std::forward<T>(cmd), in);
}
