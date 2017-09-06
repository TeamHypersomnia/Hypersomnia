#pragma once
#include "application/debug_settings.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "view/debug_character_selection.h"
#include "augs/misc/debug_entropy_player.h"
#include "game/assets/all_logical_assets.h"
#include "augs/misc/fixed_delta_timer.h"
#include "game/organization/all_component_includes.h"

class editor_setup {
public:
	cosmos subject_cosmos;
	cosmic_entropy total_collected_entropy;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);
	entity_id viewed_entity_id;
	all_logical_assets logical_assets;

	editor_setup(const std::string& directory);

	void save(const std::string& directory);
	void load(const std::string& directory);

	auto get_audiovisual_speed() const {
		return timer.get_stepping_speed_multiplier();
	}

	auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(subject_cosmos.get_fixed_delta());
	}

	entity_handle get_viewed_character() {
		return subject_cosmos[viewed_entity_id];
	}

	const auto& get_viewing_cosmos() const {
		return subject_cosmos;
	}

	template <class F, class G>
	void advance(
		F&& advance_audiovisuals, 
		G&& step_post_solve
	) {
		auto steps = timer.count_logic_steps_to_perform(subject_cosmos.get_fixed_delta());

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

	void control(
		const cosmic_entropy&
	);
};