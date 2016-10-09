#pragma once
#include "step_packaged_for_network.h"
#include "augs/misc/jitter_buffer.h"
#include "game/components/transform_component.h"

namespace augs {
	namespace network {
		class client;
	}
}

class simulation_receiver {
	struct misprediction_candidate_entry {
		entity_id id;
		components::transform transform;
	};

	struct step_to_simulate {
		bool resubstantiate = false;
		guid_mapped_entropy entropy;
	};

	struct unpacking_result {
		std::vector<step_to_simulate> entropies_to_simulate;
		bool reconciliate_predicted = false;
	};

	misprediction_candidate_entry acquire_potential_misprediction(const const_entity_handle&) const;
	std::vector<misprediction_candidate_entry> acquire_potential_mispredictions(const cosmos& predicted_cosmos_before_reconciliation) const;

	unpacking_result unpack_deterministic_steps(cosmos& referential_cosmos, cosmos& last_delta_unpacked);
	void drag_mispredictions_into_past(const cosmos& predicted_cosmos, const std::vector<misprediction_candidate_entry>& mispredictions) const;
public:
	augs::jitter_buffer<step_packaged_for_network> jitter_buffer;
	std::vector<guid_mapped_entropy> predicted_steps;

	float resubstantiate_prediction_every_ms = 1000;

	void acquire_next_packaged_step(const step_packaged_for_network&);

	template<class Step>
	void send_commands_and_predict(augs::network::client& client, cosmic_entropy new_entropy, cosmos& predicted_cosmos, Step advance) {
		augs::stream client_commands;
		augs::write_object(client_commands, network_command::CLIENT_REQUESTED_ENTROPY);

		guid_mapped_entropy guid_mapped(new_entropy, predicted_cosmos);
		augs::write_object(client_commands, guid_mapped);

		client.post_redundant(client_commands);

		predicted_steps.push_back(guid_mapped);
		advance(new_entropy, predicted_cosmos);
	}

	template<class Step>
	unpacking_result unpack_deterministic_steps(cosmos& referential_cosmos, cosmos& last_delta_unpacked, cosmos& predicted_cosmos, Step advance) {
		auto result = unpack_deterministic_steps(referential_cosmos, last_delta_unpacked);
		auto& reconciliate_predicted = result.reconciliate_predicted;

		for (const auto& e : result.entropies_to_simulate) {
			const cosmic_entropy cosmic_entropy_for_this_step(e.entropy, referential_cosmos);
			
			if (e.resubstantiate)
				referential_cosmos.complete_resubstantiation();

			advance(cosmic_entropy_for_this_step, referential_cosmos);

			if (0 == referential_cosmos.get_total_steps_passed() % static_cast<int>(resubstantiate_prediction_every_ms / referential_cosmos.get_fixed_delta().in_milliseconds())) {
				reconciliate_predicted = true;
			}
		}
		
		// LOG("Unpacking from %x to %x", previous_step, referential_cosmos.get_total_steps_passed());

		if (reconciliate_predicted) {
			const auto potential_mispredictions = acquire_potential_mispredictions(predicted_cosmos);

			predicted_cosmos = referential_cosmos;

			for (const auto& s : predicted_steps) {
				advance(cosmic_entropy(s, predicted_cosmos), predicted_cosmos);
			}

			drag_mispredictions_into_past(predicted_cosmos, potential_mispredictions);
		}

		return std::move(result);
	}
};