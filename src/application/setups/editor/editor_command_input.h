#pragma once

struct editor_folder;
class editor_entity_selector;

namespace sol {
	class state;
}

class editor_entity_mover;
struct editor_fae_gui;

class cosmos;

struct entity_id;

struct editor_command_input {
	sol::state& lua;
	editor_folder& folder;
	editor_entity_selector& selector;

	editor_fae_gui& fae_gui;
	editor_entity_mover& mover;

	cosmos& get_cosmos() const; 

	void purge_selections() const;
	void interrupt_tweakers() const;
	void clear_selection_of(entity_id) const;
};

template <class E, class T>
void post_editor_command(const E& in, T&& cmd) {
	in.folder.history.execute_new(std::forward<T>(cmd), in);
}
