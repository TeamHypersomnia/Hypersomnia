#pragma once
#include <unordered_set>

#include "augs/network/jitter_buffer.h"

#include "game/cosmos/cosmic_functions.h"

#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/past_infection_system.h"

#include "application/network/server_step_entropy.h"
#include "application/network/simulation_receiver_settings.h"

struct misprediction_candidate_entry {
	entity_id id;
	components::transform transform;
};

struct steps_unpacking_result {
	bool should_repredict = false;
	bool malicious_server = false;
};

class simulation_receiver {
public:
	using received_entropy_type = server_step_entropy_for_client;
	using simulated_entropy_type = server_step_entropy;
private:

	template <class H>
	misprediction_candidate_entry acquire_potential_misprediction(const H& e) const {
		misprediction_candidate_entry candidate;
		
		if (const auto t = e.find_logic_transform()) {
			candidate.transform = *t;
		}

		candidate.id = e.get_id();

		return candidate;
	}
	
	std::vector<misprediction_candidate_entry> acquire_potential_mispredictions(
		const std::unordered_set<entity_id>&, 
		const cosmos& predicted_cosmos_before_reconciliation
	) const;

	void drag_mispredictions_into_past(
		const simulation_receiver_settings&,
		interpolation_system&,
		past_infection_system&, 
		const cosmos& predicted_arena, 
		const std::vector<misprediction_candidate_entry>& mispredictions
	) const;

	void predict_intents_of_remote_entities(
		simulated_entropy_type& adjusted_entropy, 
		const entity_id locally_controlled_entity, 
		const cosmos& predicted_arena
	);

public:

	std::vector<received_entropy_type> incoming_entropies;
	std::vector<simulated_entropy_type> predicted_entropies;

	void acquire_next_server_entropy(const received_entropy_type& step) {
		incoming_entropies.push_back(step);
	}

	template <class A, class S1, class S2>
	steps_unpacking_result unpack_deterministic_steps(
		const simulation_receiver_settings& settings,

		interpolation_system& interp, 
		past_infection_system& past,

		const entity_id locally_controlled_entity, 

		A& referential_arena, 
		A& predicted_arena, 

		S1 advance_referential,
		S2 advance_predicted
	) {
		//auto result = unpack_deterministic_steps(referential_arena);
		steps_unpacking_result result;

		//std::size_t total_accepted = 0;

		auto& repredict = result.should_repredict;

		{
			auto p_i = std::size_t(0);

			for (std::size_t i = 0; i < incoming_entropies.size(); ++i) {
				const auto& received = incoming_entropies[i];

				/* If a new player was added, always reinfer. */
				const auto& actual_server_step = received.total;
				const bool shall_reinfer = actual_server_step.general.added_player != std::nullopt;

				{
					auto& referential_cosmos = referential_arena.get_cosmos();

					if (shall_reinfer) {
						cosmic::reinfer_solvable(referential_cosmos);
					}

					advance_referential(actual_server_step);
				}

				if (!repredict) {
					const auto& predicted_server_step = predicted_entropies[p_i];

					if (shall_reinfer || !(actual_server_step == predicted_server_step)) {
						repredict = true;
					}
				}

				{
					const auto num_accepted = received.num_entropies_accepted;

					if (num_accepted != 1) {
						/* We'll need to nudge into the future or into the past. */

						repredict = true;
					}

					p_i += num_accepted;
				}

			}

			const auto& total_accepted = p_i;

			auto& p = predicted_entropies;

			if (total_accepted < p.size()) {
				p.erase(p.begin(), p.begin() + total_accepted);
			}
			else {
				result.malicious_server = true;
				return result;
			}
		}

		if (repredict) {
			auto& predicted_cosmos = predicted_arena.get_cosmos();

			const auto potential_mispredictions = acquire_potential_mispredictions(
				past.infected_entities, 
				predicted_cosmos
			);

			predicted_arena.assign_all_solvables(referential_arena);

			for (auto& predicted_step_entropy : predicted_entropies) {
				predict_intents_of_remote_entities(
					predicted_step_entropy,
					locally_controlled_entity, 
					predicted_cosmos
				);

				advance_predicted(predicted_step_entropy);
			}

			drag_mispredictions_into_past(
				settings, 
				interp, 
				past, 
				predicted_cosmos, 
				potential_mispredictions
			);
		}

		return result;
	}
};