#pragma once
#include <future>

#include "augs/misc/fixed_delta_timer.h"
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
};

class editor_setup {
	struct popup {
		std::string title;
		std::string message;
		std::string details;

		bool details_expanded = false;
	};
	
	std::optional<popup> current_popup;

	bool show_summary = true;
	bool show_player = true;
	bool show_common_state = false;
	bool show_entities = false;
	bool show_go_to_all = false;

	double player_speed = 1.0;
	bool player_paused = true;

	void play();
	void pause();

	workspace work;
	editor_recent_paths recent;

	cosmic_entropy total_collected_entropy;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };

	augs::path_type current_workspace_path;

	std::future<std::optional<std::string>> open_file_dialog;
	std::future<std::optional<std::string>> save_file_dialog;

	void set_popup(const popup);
	void open_workspace(sol::state&, const augs::path_type& workspace_path);
	void save_workspace(sol::state&, const augs::path_type& workspace_path);
	void set_workspace_path(sol::state&, const augs::path_type&);
	void open_untitled_workspace();

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool accepts_media_keys = true;
	static constexpr bool accepts_shortcuts = true;
	static constexpr bool has_modal_popups = true;

	editor_setup(sol::state& lua); // Loads the most recent work
	editor_setup(sol::state& lua, const augs::path_type& workspace_path);

	auto get_audiovisual_speed() const {
		return player_paused ? 0.0 : player_speed;
	}

	const auto& get_viewed_cosmos() const {
		return work.world;
	}

	auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(get_viewed_cosmos().get_fixed_delta());
	}

	auto get_viewed_character_id() const {
		return work.locally_viewed;
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return work.viewables;
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
			work.advance(
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

	void go_to_all();
	void open_containing_folder();
};