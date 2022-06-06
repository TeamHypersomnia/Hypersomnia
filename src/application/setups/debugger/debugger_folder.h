#pragma once
#include <string>
#include <vector>

#include "augs/filesystem/path.h"
#include "augs/graphics/vertex.h"

#include "application/setups/debugger/debugger_history.h"
#include "application/setups/debugger/debugger_player.h"
#include "application/setups/debugger/debugger_commanded_state.h"

using folder_index = unsigned;
constexpr unsigned dead_folder_v = static_cast<folder_index>(-1);

struct simple_popup;
using debugger_warning = simple_popup;

namespace sol {
	class state;
}

struct debugger_recent_paths;
struct debugger_paths;

enum class debugger_save_type {
	EVERYTHING,
	ONLY_VIEW
};

struct debugger_folder {
	debugger_folder(const augs::path_type& p = {});

	augs::path_type current_path;

	std::unique_ptr<debugger_commanded_state> commanded;

	debugger_view view;
	debugger_player player;
	debugger_history history;

	/* Opened game mode definitions go here */

	void set_folder_path(const augs::path_type&);
	std::string get_display_path() const;

	bool is_untitled() const;

	void save_folder(debugger_save_type = debugger_save_type::EVERYTHING) const;
	void export_folder(sol::state& lua, const augs::path_type& to) const;

	std::optional<debugger_warning> open_most_relevant_content(sol::state& lua);

	void mark_as_just_saved();
	bool empty() const;

	bool allow_close() const;

	debugger_paths get_paths() const;

	entity_id get_controlled_character_id() const;

	void autosave_if_needed() const;

	double get_inv_tickrate() const;
	double get_audiovisual_speed() const;

private:
	void import_folder(sol::state& lua, const augs::path_type& from);

	void load_folder();
	void load_folder(const augs::path_type& from);
	void load_folder(const augs::path_type& from, const augs::path_type& name);

	void save_folder(const augs::path_type& to, debugger_save_type = debugger_save_type::EVERYTHING) const;
	void save_folder(const augs::path_type& to, const augs::path_type name, debugger_save_type = debugger_save_type::EVERYTHING) const;

	bool should_autosave() const;
	augs::path_type get_autosave_path() const;
};

struct debugger_last_folders {
	// GEN INTROSPECTOR struct debugger_last_folders
	folder_index current_index = static_cast<folder_index>(-1);
	std::vector<augs::path_type> paths;
	// END GEN INTROSPECTOR
};