#pragma once
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

struct config_lua_table;

namespace sol {
	class state;
}

class test_scene_setup {
	workspace scene;
	cosmic_entropy total_collected_entropy;
	augs::debug_entropy_player<cosmic_entropy> player;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };
	debug_character_selection characters;

public:
	static constexpr auto loading_strategy = viewables_loading_type::ALWAYS_HAVE_ALL_LOADED;
	static constexpr bool can_viewables_change = false;
	static constexpr bool accepts_shortcuts = false;
	static constexpr bool has_modal_popups = false;

	test_scene_setup(
		sol::state& lua,
		const bool make_minimal_test_scene,
		const input_recording_type recording_type
	);

	auto get_audiovisual_speed() const {
		return 1.0;
	}

	const auto& get_viewed_cosmos() const {
		return scene.world;
	}

	auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(get_viewed_cosmos().get_fixed_delta());
	}

	auto get_viewed_character_id() const {
		return characters.get_selected_character();
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return scene.viewables;
	}

	void perform_custom_imgui() {
		return;
	}

	void customize_for_viewing(config_lua_table&) const;

	void apply(const config_lua_table&) {
		return;
	}

	template <class... Callbacks>
	void advance(
		const augs::delta frame_delta,
		Callbacks&&... callbacks
	) {
		timer.advance(frame_delta);
		auto steps = timer.extract_num_of_logic_steps(get_viewed_cosmos().get_fixed_delta());

		while (steps--) {
			player.advance_player_and_biserialize(total_collected_entropy);

			scene.advance(
				{ total_collected_entropy },
				std::forward<Callbacks>(callbacks)...
			);

			total_collected_entropy.clear();
		}
	}

	void control(const cosmic_entropy&);
	void accept_game_gui_events(const cosmic_entropy&);
};