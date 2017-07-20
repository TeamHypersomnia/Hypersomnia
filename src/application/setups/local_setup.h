#pragma once
#include "application/debug_settings.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/view/debug_character_selection.h"
#include "augs/misc/debug_entropy_player.h"
#include "game/assets/assets_manager.h"
#include "augs/misc/fixed_delta_timer.h"
#include "game/transcendental/types_specification/all_component_includes.h"

class local_setup {
	cosmos hypersomnia;
	cosmic_entropy total_collected_entropy;
	augs::debug_entropy_player<cosmic_entropy> player;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);
	debug_character_selection characters;
	all_logical_metas_of_assets metas_of_assets;
public:
	template <class P>
	local_setup(
		P&& populate,
		const input_recording_type recording_type
	) {
		populate(hypersomnia, metas_of_assets);
		characters.acquire_available_characters(hypersomnia);

		if (recording_type != input_recording_type::DISABLED) {
			if (player.try_to_load_or_save_new_session("generated/sessions/", "recorded.inputs")) {
				timer.set_stepping_speed_multiplier(1.f);
			}
		}

		timer.reset_timer();
	}

	auto get_audiovisual_speed() const {
		return timer.get_stepping_speed_multiplier();
	}

	auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(hypersomnia.get_fixed_delta());
	}

	entity_handle get_selected_character() {
		return hypersomnia[characters.get_selected_character()];
	}

	const auto& get_viewing_cosmos() const {
		return hypersomnia;
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

			hypersomnia.advance_deterministic_schemata(
				{ total_collected_entropy, metas_of_assets },
				[](auto){},
				std::forward<G>(step_post_solve)
			);

			total_collected_entropy.clear();
			advance_audiovisuals();
		}
	}

	void control(
		augs::machine_entropy::local_type& entropy,
		const input_context&
	);

	void control(
		const cosmic_entropy&
	);
};