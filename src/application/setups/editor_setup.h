#pragma once
#include <future>
#include <map>

#include "augs/misc/timing/fixed_delta_timer.h"
#include "augs/misc/debug_entropy_player.h"

#include "game/assets/all_logical_assets.h"

#include "game/organization/all_component_includes.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "view/viewables/all_viewables_defs.h"
#include "view/viewables/viewables_loading_type.h"

#include "application/workspace.h"

#include "application/debug_character_selection.h"
#include "application/debug_settings.h"
#include "application/setups/editor_settings.h"

struct config_lua_table;

namespace sol {
	class state;
}

namespace augs {
	class window;
}

struct editor_recent_paths {
	// GEN INTROSPECTOR struct editor_recent_paths
	std::vector<augs::path_type> paths;
	// END GEN INTROSPECTOR

	editor_recent_paths(sol::state& lua);
	void add(sol::state&, const augs::path_type& path);
	void clear(sol::state&);
	bool empty() const;
};

struct editor_popup {
	std::string title;
	std::string message;
	std::string details;

	bool details_expanded = false;
};

struct editor_tab {
	workspace work;
	augs::path_type current_path;
	std::size_t horizontal_index;

	struct path_operation {
		sol::state& lua;
		editor_recent_paths& recent;
		const augs::path_type& path;
	};

	editor_tab(std::size_t horizontal_index);

	std::optional<editor_popup> open_workspace(path_operation);
	
	void save_workspace(path_operation);
	void set_workspace_path(path_operation);
};

using editor_tab_container = std::map<std::size_t, editor_tab>;

class editor_setup {
	std::optional<editor_popup> current_popup;

	bool show_summary = true;
	bool show_player = true;
	bool show_common_state = false;
	bool show_entities = false;
	bool show_go_to_all = false;

	double player_speed = 1.0;
	bool player_paused = true;

	editor_recent_paths recent;

	editor_tab_container tabs;
	editor_tab* current_tab = nullptr;

	void set_current_tab(editor_tab& t) {
		current_tab = std::addressof(t);
		pause();
	}

	void unset_current_tab() {
		current_tab = nullptr;
		pause();
	}

	auto& tab() {
		return *current_tab;
	}

	const auto& tab() const {
		return *current_tab;
	}

	bool has_tabs() const {
		return !tabs.empty();
	}

	void set_tab_by_index(const std::size_t);

	template <class F>
	void try_new_tab(F&& f) {
		const auto new_id = first_free_key(tabs);
		const auto new_horizontal_index = current_tab ? current_tab->horizontal_index + 1 : 0;

		auto& new_tab = (*tabs.try_emplace(new_id, new_horizontal_index).first).second;
		
		if (f(new_tab)) {
			set_current_tab(new_tab);

			for (auto& it : tabs) {
				if (current_tab != std::addressof(it.second) 
					&& it.second.horizontal_index >= new_horizontal_index
				) {
					++it.second.horizontal_index;
				}
			}
		}
		else {
			tabs.erase(new_id);
		}
	}

	void play();
	void pause();

	cosmic_entropy total_collected_entropy;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };

	std::future<std::optional<std::string>> open_file_dialog;
	std::future<std::optional<std::string>> save_file_dialog;

	void set_popup(const editor_popup);
	void open_untitled_workspace();
	
	struct path_operation {
		sol::state& lua;
		const augs::path_type path;
	};

	void open_workspace(path_operation);
	void save_workspace(path_operation);

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool accepts_media_keys = true;
	static constexpr bool accepts_shortcuts = true;
	static constexpr bool has_modal_popups = true;

	editor_setup(sol::state& lua);
	editor_setup(sol::state& lua, const augs::path_type& workspace_path);

	auto get_audiovisual_speed() const {
		return player_paused ? 0.0 : player_speed;
	}

	const auto& get_viewed_cosmos() const {
		if (has_tabs()) {
			return tab().work.world;
		}
		
		return cosmos::empty; 
	}

	auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(get_viewed_cosmos().get_fixed_delta());
	}

	auto get_viewed_character_id() const {
		return has_tabs() ? tab().work.locally_viewed : entity_id();
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		if (has_tabs()) {
			return tab().work.viewables;
		}

		return all_viewables_defs::empty;
	}

	void perform_custom_imgui(
		sol::state& lua,
		augs::window& owner,
		const bool game_gui_active
	);

	bool during_popup() const {
		return current_popup.has_value();
	}

	void customize_for_viewing(config_lua_table& cfg) const;

	void apply(const config_lua_table&) {
		return;
	}

	template <class... Callbacks>
	void advance(
		augs::delta frame_delta,
		Callbacks&&... callbacks
	) {
		if (!player_paused) {
			timer.advance(frame_delta *= player_speed);
		}

		auto steps = timer.extract_num_of_logic_steps(get_viewed_cosmos().get_fixed_delta());

		while (steps--) {
			tab().work.advance(
				{ total_collected_entropy },
				std::forward<Callbacks>(callbacks)...
			);

			total_collected_entropy.clear();
		}
	}

	void control(const cosmic_entropy&);
	void accept_game_gui_events(const cosmic_entropy&);

	bool escape_modal_popup();
	bool confirm_modal_popup();

	void open(const augs::window& owner);
	void save(sol::state& lua, const augs::window& owner);
	void save_as(const augs::window& owner);
	void undo();
	void redo();

	void copy();
	void cut();
	void paste();

	void play_pause();
	void stop();
	void prev();
	void next();

	void new_tab();
	void next_tab();
	void prev_tab();
	void close_tab();

	void go_to_all();
	void open_containing_folder();
};