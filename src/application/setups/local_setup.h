#pragma once
#include "augs/misc/fixed_delta_timer.h"
#include "augs/misc/debug_entropy_player.h"

#include "game/assets/all_assets.h"
#include "game/view/debug_character_selection.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "application/debug_settings.h"

struct config_lua_table;

class local_setup {
	cosmos hypersomnia;
	cosmic_entropy total_collected_entropy;
	augs::debug_entropy_player<cosmic_entropy> player;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);
	debug_character_selection characters;

	all_logical_assets logical_assets;
	all_viewable_defs viewable_defs;

public:
	static constexpr auto loading_strategy = viewables_loading_type::ALWAYS_HAVE_ALL_LOADED;
	static constexpr bool can_viewables_change = false;

	local_setup(
		const bool make_minimal_test_scene,
		const input_recording_type recording_type
	);

	auto get_audiovisual_speed() const {
		return timer.get_stepping_speed_multiplier();
	}

	auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(hypersomnia.get_fixed_delta());
	}

	auto get_viewed_character_id() const {
		return characters.get_selected_character();
	}

	const auto& get_viewed_cosmos() const {
		return hypersomnia;
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return viewable_defs;
	}

	void perform_custom_imgui() {
		return;
	}

	void customize_for_viewing(config_lua_table&) {
		return;
	}

	void apply(const config_lua_table&) {
		return;
	}

	template <class F, class G>
	void advance(
		F&& advance_audiovisuals, 
		G&& step_post_solve
	) {
		auto steps = timer.count_logic_steps_to_perform(hypersomnia.get_fixed_delta());

		if (!steps) {
			advance_audiovisuals();
		}

		while (steps--) {
			player.advance_player_and_biserialize(total_collected_entropy);

			augs::renderer::get_current().clear_logic_lines();

			hypersomnia.advance(
				{ total_collected_entropy, logical_assets },
				[](auto){},
				std::forward<G>(step_post_solve)
			);

			total_collected_entropy.clear();
			advance_audiovisuals();
		}
	}

	void control(
		augs::local_entropy& entropy,
		const input_context&
	);

	void accept_game_gui_events(const cosmic_entropy&);
};