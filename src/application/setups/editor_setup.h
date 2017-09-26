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

#include "application/debug_character_selection.h"
#include "application/debug_settings.h"
#include "application/setups/editor_settings.h"

struct config_lua_table;

namespace sol {
	class state;
}

class editor_setup {
	cosmos edited_world;
	cosmic_entropy total_collected_entropy;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };
	entity_id viewed_character_id;

	all_logical_assets logical_assets;
	all_viewables_defs viewable_defs;

	struct popup {
		std::string title;
		std::string message;
		std::string details;

		bool details_expanded = false;
	};

	std::optional<popup> current_popup;
	augs::path_type current_cosmos_path;

	std::future<std::string> open_file_dialog;

	void set_popup(const popup);
	void open_cosmos(const augs::path_type& cosmos_path);
	void start_open_file_dialog();

public:
	static constexpr auto loading_strategy = viewables_loading_type::ALWAYS_HAVE_ALL_LOADED;
	static constexpr bool can_viewables_change = false;

	editor_setup(const augs::path_type& cosmos_path);

	auto get_audiovisual_speed() const {
		return 1.0;
	}

	auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(edited_world.get_fixed_delta());
	}

	auto get_viewed_character_id() const {
		return viewed_character_id;
	}

	const auto& get_viewed_cosmos() const {
		return edited_world;
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return viewable_defs;
	}

	void perform_custom_imgui();

	bool during_popup() const {
		return current_popup.has_value();
	}

	void customize_for_viewing(config_lua_table&) {
		return;
	}

	void apply(const config_lua_table&) {
		return;
	}

	template <class... Callbacks>
	void advance(
		const augs::delta frame_delta,
		Callbacks&&... callbacks
	) {
		timer.advance(frame_delta);
		auto steps = timer.extract_num_of_logic_steps(edited_world.get_fixed_delta());

		while (steps--) {
			edited_world.advance(
			{ total_collected_entropy, logical_assets },
				std::forward<Callbacks>(callbacks)...
			);

			total_collected_entropy.clear();
		}
	}

	void control(const cosmic_entropy&);
	void accept_game_gui_events(const cosmic_entropy&);
};