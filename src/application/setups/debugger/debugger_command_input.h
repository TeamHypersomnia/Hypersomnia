#pragma once
#include <utility>
#include "augs/templates/snapshotted_player_step_type.h"

struct debugger_folder;
class debugger_entity_selector;

namespace sol {
	class state;
}

class debugger_entity_mover;
struct debugger_fae_gui;
struct debugger_selected_fae_gui;

class cosmos;

struct entity_id;
struct all_viewables_defs;
struct all_logical_assets;
struct debugger_history;
struct debugger_settings;
class debugger_player;

struct debugger_command_input {
	sol::state& lua;
	const debugger_settings& settings;
	debugger_folder& folder;
	debugger_entity_selector& selector;

	debugger_fae_gui& fae_gui;
	debugger_selected_fae_gui& selected_fae_gui;
	debugger_entity_mover& mover;

	cosmos& get_cosmos() const; 

	debugger_history& get_history() const;
	debugger_player& get_player() const;
	all_viewables_defs& get_viewable_defs() const;
	const all_logical_assets& get_logical_assets() const;

	augs::snapshotted_player_step_type get_current_step() const;

	void purge_selections() const;
	void interrupt_tweakers() const;
	void clear_dead_entity(entity_id) const;
	void clear_dead_entities() const;

	bool allow_new_commands() const;

	static debugger_command_input make_dummy_for(sol::state&, debugger_folder&);
};

template <class E, class T>
const T& post_debugger_command(const E& in, T&& cmd) {
	return in.get_history().execute_new(std::forward<T>(cmd), in);
}
