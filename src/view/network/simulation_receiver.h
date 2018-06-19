#pragma once
#include <unordered_set>

#include "augs/network/network_client.h"
#include "augs/network/jitter_buffer.h"

#include "game/components/transform_component.h"
#include "game/transcendental/cosmos.h"

#include "view/network/network_commands.h"
#include "view/network/step_packaged_for_network.h"
#include "view/network/simulation_receiver_settings.h"

#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/past_infection_system.h"

struct misprediction_candidate_entry {
	entity_id id;
	transformr transform;
};

struct step_to_simulate {
	bool reinfer = false;
	guid_mapped_entropy entropy;
};

struct steps_unpacking_result {
	std::vector<step_to_simulate> entropies_to_simulate;
	bool reconciliate_predicted = false;
};

class simulation_receiver {
	misprediction_candidate_entry acquire_potential_misprediction(const const_entity_handle) const;
	
	std::vector<misprediction_candidate_entry> acquire_potential_mispredictions(
		const std::unordered_set<entity_id>&, 
		const cosmos& predicted_cosmos_before_reconciliation
	) const;

	steps_unpacking_result unpack_deterministic_steps(cosmos& referential_cosmos, cosmos& last_delta_unpacked);
	void drag_mispredictions_into_past(
		const simulation_receiver_settings&,
		interpolation_system&,
		past_infection_system&, 
		const cosmos& predicted_cosmos, 
		const std::vector<misprediction_candidate_entry>& mispredictions
	) const;

	void predict_intents_of_remote_entities(
		guid_mapped_entropy& adjusted_entropy, 
		const entity_id locally_controlled_entity, 
		const cosmos& predicted_cosmos
	);
public:
	augs::jitter_buffer<step_packaged_for_network> jitter_buffer;
	std::vector<guid_mapped_entropy> predicted_step_entropies;

	float reinfer_prediction_every_ms = 1000.f;

	void acquire_next_packaged_step(const step_packaged_for_network&);

	template<class Step>
	void send_commands_and_predict(
		augs::network::client& client, 
		const cosmic_entropy& new_local_entropy, 
		cosmos& predicted_cosmos, 
		Step advance
	) {
		guid_mapped_entropy guid_mapped(new_local_entropy, predicted_cosmos);

		augs::memory_stream client_commands;
		augs::write_bytes(client_commands, network_command::CLIENT_REQUESTED_ENTROPY);
		augs::write_bytes(client_commands, guid_mapped);

		client.post_redundant(client_commands);

		predicted_step_entropies.push_back(guid_mapped);
		advance(new_local_entropy, predicted_cosmos);
	}

	template<class Step>
	steps_unpacking_result unpack_deterministic_steps(
		const simulation_receiver_settings& settings,
		interpolation_system& interp, 
		past_infection_system& past,
		const entity_id locally_controlled_entity, 
		cosmos& referential_cosmos, 
		cosmos& last_delta_unpacked, 
		cosmos& predicted_cosmos, 
		Step advance
	) {
		auto result = unpack_deterministic_steps(referential_cosmos, last_delta_unpacked);
		auto& reconciliate_predicted = result.reconciliate_predicted;

		for (const auto& e : result.entropies_to_simulate) {
			const cosmic_entropy cosmic_entropy_for_this_step(e.entropy, referential_cosmos);
			
			if (e.reinfer) {
				LOG("Cli: %x resubs at step: %x", locally_controlled_entity, referential_cosmos.get_total_steps_passed());
				cosmic::reinfer_all_entities(referential_cosmos);
			}

			advance(cosmic_entropy_for_this_step, referential_cosmos);

			const auto total_steps = referential_cosmos.get_total_steps_passed();
			const auto reinfer_once_per_steps = static_cast<int>(reinfer_prediction_every_ms / referential_cosmos.get_fixed_delta().in_milliseconds());

			if (total_steps % reinfer_once_per_steps == 0) {
				reconciliate_predicted = true;
			}
		}
		
		// LOG("Unpacking from %x to %x", previous_step, referential_cosmos.get_total_steps_passed());

		if (reconciliate_predicted) {
			const auto potential_mispredictions = acquire_potential_mispredictions(past.infected_entities, predicted_cosmos);

			predicted_cosmos = referential_cosmos;

			for (auto& predicted_step_entropy : predicted_step_entropies) {
				predict_intents_of_remote_entities(
					predicted_step_entropy,
					locally_controlled_entity, 
					predicted_cosmos
				);

				advance(
					cosmic_entropy(predicted_step_entropy, predicted_cosmos),
					predicted_cosmos
				);
			}

			drag_mispredictions_into_past(settings, interp, past, predicted_cosmos, potential_mispredictions);
		}

		return result;
	}
};