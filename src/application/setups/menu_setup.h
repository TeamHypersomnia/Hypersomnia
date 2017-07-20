#pragma once
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/assets/assets_manager.h"
#include "augs/misc/fixed_delta_timer.h"
#include "game/transcendental/types_specification/all_component_includes.h"

class menu_setup {
	cosmos hypersomnia;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);
	all_logical_metas_of_assets metas_of_assets;
public:
	template <class P>
	menu_setup(P&& populate) {
		populate(hypersomnia, metas_of_assets);
		
		timer.set_stepping_speed_multiplier(1.f);
		timer.reset_timer();
	}

	auto get_audiovisual_speed() const {
		return timer.get_stepping_speed_multiplier();
	}

	auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(hypersomnia.get_fixed_delta());
	}

	entity_handle get_selected_character() {
		return hypersomnia.get_entity_by_name(L"player0");
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
			hypersomnia.advance_deterministic_schemata(
			{ total_collected_entropy, metas_of_assets },
				[](auto) {},
				std::forward<G>(step_post_solve)
			);

			total_collected_entropy.clear();
			advance_audiovisuals();
		}
	}
};