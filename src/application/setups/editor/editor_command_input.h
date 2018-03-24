#pragma once

class editor_folder;
class editor_entity_selector;

namespace sol {
	class state;
}
struct editor_all_entities_gui;

struct editor_command_input {
	sol::state& lua;
	editor_folder& folder;
	editor_entity_selector& selector;

	editor_all_entities_gui& all_entities_gui;

	void purge_selections() const;
	void interrupt_tweakers() const;
};
