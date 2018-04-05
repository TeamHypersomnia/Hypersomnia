#pragma once

class editor_folder;
class editor_entity_selector;

namespace sol {
	class state;
}

struct editor_entity_mover;
struct editor_all_entities_gui;

struct editor_entity_mover;

class cosmos;

struct entity_id;

struct editor_command_input {
	sol::state& lua;
	editor_folder& folder;
	editor_entity_selector& selector;

	editor_all_entities_gui& all_entities_gui;
	editor_entity_mover& mover;

	cosmos& get_cosmos() const; 

	void purge_selections() const;
	void interrupt_tweakers() const;
	void clear_selection_of(entity_id) const;
};
