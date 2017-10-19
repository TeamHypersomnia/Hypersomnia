#pragma once
#include <future>
#include <map>
#include <unordered_map>

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

	namespace event {
		struct change;
		struct state;
	}
}

struct editor_recent_paths {
	// GEN INTROSPECTOR struct editor_recent_paths
	std::vector<augs::path_type> paths;
	// END GEN INTROSPECTOR

	editor_recent_paths(sol::state& lua);
	void add(const workspace_path_op);
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
	// GEN INTROSPECTOR struct editor_tab
	augs::path_type current_path;
	std::unordered_map<entity_id, rgba> selected_entities;
	vec2 panning;
	// END GEN INTROSPECTOR

	void set_workspace_path(const workspace_path_op, editor_recent_paths&);
	
	bool has_unsaved_changes() const;
	bool is_untitled() const;

	std::string get_display_path() const;
};

struct editor_saved_tabs {
	// GEN INTROSPECTOR struct editor_saved_tabs
	std::size_t current_tab_index = -1;
	std::vector<editor_tab> tabs;
	// END GEN INTROSPECTOR
};

class editor_setup {
	struct autosave_input {
		sol::state& lua;
	};

	const autosave_input destructor_autosave_input;

	std::optional<editor_popup> current_popup;

	bool show_summary = true;
	bool show_player = true;
	bool show_common_state = false;
	bool show_entities = false;
	bool show_go_to_all = false;

	double player_speed = 1.0;
	bool player_paused = true;

	editor_recent_paths recent;
	
	std::vector<editor_tab> tabs;
	std::vector<std::unique_ptr<workspace>> works;

	std::size_t current_index = -1;
	
	/* Cache for fast access */
	editor_tab* current_tab = nullptr;
	workspace* current_work = nullptr;

	void set_current_tab(const std::size_t i) {
		current_index = i;

		current_tab = &tabs[i];
		current_work = works[i].get();
	}

	void unset_current_tab() {
		current_index = -1;

		current_tab = nullptr;
		current_work = nullptr;
	};

	void set_current_tab(editor_tab& t) {
		current_tab = std::addressof(t);
		pause();
	}

	bool has_current_tab() const {
		return current_tab != nullptr;
	}

	auto& tab() {
		return *current_tab;
	}

	const auto& tab() const {
		return *current_tab;
	}

	auto& work() {
		return *current_work;
	}

	const auto& work() const {
		return *current_work;
	}

	void set_locally_viewed(const entity_id);

	template <class F>
	bool try_to_open_new_tab(F&& new_workspace_provider) {
		const auto new_index = has_current_tab() ? current_index + 1 : 0;

		works.reserve(works.size() + 1);
		tabs.reserve(tabs.size() + 1);

		tabs.emplace(tabs.begin() + new_index);
		works.emplace(works.begin() + new_index, std::make_unique<workspace>());

		if (const bool successfully_opened = new_workspace_provider(tabs[new_index], *works[new_index])) {
			set_current_tab(new_index);
			return true;
		}

		tabs.erase(tabs.begin() + new_index);
		works.erase(works.begin() + new_index);

		return false;
	}

	void play();
	void pause();

	cosmic_entropy total_collected_entropy;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };

	std::future<std::optional<std::string>> open_file_dialog;
	std::future<std::optional<std::string>> save_file_dialog;

	void set_popup(const editor_popup);
	
	using path_operation = workspace_path_op;

	bool open_workspace_in_new_tab(path_operation);
	void save_current_tab_to(path_operation);

	void fill_with_test_scene(sol::state& lua);

	void autosave(const autosave_input) const;
	void open_last_tabs(sol::state& lua);

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;
	static constexpr bool handles_escape = true;

	editor_setup(sol::state& lua);
	editor_setup(sol::state& lua, const augs::path_type& workspace_path);
	
	~editor_setup();

	auto get_audiovisual_speed() const {
		return player_paused ? 0.0 : player_speed;
	}

	const auto& get_viewed_cosmos() const {
		return has_current_tab() ? work().world : cosmos::empty; 
	}

	auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(get_viewed_cosmos().get_fixed_delta());
	}

	auto get_viewed_character_id() const {
		return has_current_tab() ? work().locally_viewed : entity_id();
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return has_current_tab() ? work().viewables : all_viewables_defs::empty;
	}

	void perform_custom_imgui(
		sol::state& lua,
		augs::window& owner,
		const bool in_direct_gameplay
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
			total_collected_entropy.clear_dead_entities(work().world);

			work().advance(
				{ total_collected_entropy },
				std::forward<Callbacks>(callbacks)...
			);

			total_collected_entropy.clear();
		}
	}

	void control(const cosmic_entropy&);
	void accept_game_gui_events(const cosmic_entropy&);

	bool handle_top_level_window_input(
		const augs::event::state& common_input_state,
		const augs::event::change change,

		augs::window& window,
		sol::state& lua
	);

	bool handle_unfetched_window_input(
		const augs::event::state& common_input_state,
		const augs::event::change change,

		augs::window& window,
		sol::state& lua
	);

	bool escape();
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
	void close_tab(const std::size_t i);

	void go_to_all();
	void open_containing_folder();

	auto get_camera_panning() const {
		return has_current_tab() ? tab().panning : vec2::zero;
	}
};