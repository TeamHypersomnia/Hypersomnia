#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/past_infection_system.h"

#include "application/network/simulation_receiver.h"

void simulation_receiver::predict_intents_of_remote_entities(
	simulation_receiver::simulated_entropy_type& adjusted_entropy, 
	const entity_id locally_controlled_entity, 
	const cosmos& predicted_cosmos
) {
	(void)adjusted_entropy;
	(void)locally_controlled_entity;
	(void)predicted_cosmos;

#if 0
	predicted_cosmos.for_each(
		processing_subjects::WITH_ENABLED_PAST_CONTAGIOUS,
		[&](const auto e) {
			const bool is_locally_controlled_entity = e == locally_controlled_entity;
			
			if (is_locally_controlled_entity) {
				return;
			}

			for (const auto g_id : e.get_wielded_guns()) {
				const auto g = const_entity_handle(predicted_cosmos[g_id]);

				if (g.get<components::gun>().is_trigger_pressed) {
					const auto slot = g.get_current_slot();

					const auto hand_index = slot.get_hand_index();

					if (slot.alive() && hand_index != 0xdeadbeef) {
						game_intent release_intent;

						if (hand_index == 0) {
							release_intent.change = intent_change::RELEASED;
							release_intent.intent = game_intent_type::SHOOT;
						}
						else if (hand_index == 1) {
							release_intent.change = intent_change::RELEASED;
							release_intent.intent = game_intent_type::SHOOT;
						}
						else {
							ensure(false && "bad hand index");
						}

						if (release_intent.is_set()) {
							adjusted_entropy.intents_per_entity[e.get_id()].push_back(release_intent);
						}
					}
				}
			}
		}
	);
#endif
}

std::vector<misprediction_candidate_entry> simulation_receiver::acquire_potential_mispredictions(
	const std::unordered_set<entity_id>& unpredictables_infected, 
	const cosmos& predicted_cosmos_before_reconciliation
) const {
	std::vector<misprediction_candidate_entry> potential_mispredictions;
	
	const auto& cosmos = predicted_cosmos_before_reconciliation;
	
	potential_mispredictions.reserve(
		cosmos.get_solvable().get_count_of(processing_subjects::WITH_ENABLED_PAST_CONTAGIOUS) + unpredictables_infected.size()
	);

	cosmos.for_each_in(
		processing_subjects::WITH_ENABLED_PAST_CONTAGIOUS,
		[&](const auto& e) {
			potential_mispredictions.push_back(acquire_potential_misprediction(e));
		}
	);

	for (const auto& e : unpredictables_infected) {
		if (const auto handle = cosmos[e]) {
			handle.dispatch(
				[&](const auto& typed_handle) {
					potential_mispredictions.push_back(acquire_potential_misprediction(typed_handle));
				}
			);
		}
	}

	return potential_mispredictions;
}

void simulation_receiver::drag_mispredictions_into_past(
	const simulation_receiver_settings& settings,
	interpolation_system& interp,
	past_infection_system& past, 
	const cosmos& predicted_cosmos, 
	const std::vector<misprediction_candidate_entry>& mispredictions
) const {
	for (const auto& e : mispredictions) {
		const auto reconciliated_entity = predicted_cosmos[e.id];

		const bool identity_matches = reconciliated_entity.alive();

		if (!identity_matches) {
			continue;
		}

		const auto reconciliated_transform = reconciliated_entity.find_logic_transform();

		if (reconciliated_transform == std::nullopt) {
			continue;
		}

		reconciliated_entity.dispatch_on_having_all<invariants::interpolation>(
			[&](const auto& typed_handle) {
				const auto maybe_driver = typed_handle.template find<components::driver>();

				const bool is_contagious_agent = typed_handle.get_flag(entity_flag::IS_PAST_CONTAGIOUS);
				const bool should_smooth_rotation = !is_contagious_agent || (maybe_driver && predicted_cosmos[maybe_driver->owned_vehicle].alive());

				(void)interp;
				auto& interp_data = get_corresponding<components::interpolation>(typed_handle);

				bool misprediction_detected = false;

				const float num_predicted_steps = static_cast<float>(predicted_entropies.size());

				if ((reconciliated_transform->pos - e.transform.pos).length_sq() > 1.f) {
					interp_data.positional_slowdown_multiplier = std::max(1.f, settings.misprediction_smoothing_multiplier * num_predicted_steps);
					misprediction_detected = true;
				}

				if (should_smooth_rotation && std::abs(reconciliated_transform->rotation - e.transform.rotation) > 1.f) {
					interp_data.rotational_slowdown_multiplier = std::max(1.f, settings.misprediction_smoothing_multiplier * num_predicted_steps);
					misprediction_detected = true;
				}

				if (identity_matches || (!misprediction_detected && !is_contagious_agent)) {
					past.uninfect(typed_handle);
				}
			}
		);
	}
}
