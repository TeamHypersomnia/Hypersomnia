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
	struct mismatch_candidate_entry {
		entity_id id;
		components::transform transform;
	};

	mismatch_candidate_entry acquire_potential_mismatch(const const_entity_handle&) const;
	std::vector<mismatch_candidate_entry> acquire_potential_mismatches(const cosmos& predicted_cosmos_before_reconciliation) const;

	void drag_mismatches_into_past(const cosmos& predicted_cosmos, const std::vector<mismatch_candidate_entry>& mismatches) const;
public:
	augs::jitter_buffer<step_packaged_for_network> jitter_buffer;
	std::vector<guid_mapped_entropy> predicted_steps;

	float resubstantiate_prediction_every_ms = 1000;

	void acquire_next_packaged_step(const step_packaged_for_network&);

	struct unpacking_result {
		bool use_extrapolated_cosmos = true;
	};

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
		unpacking_result result;

		auto new_commands = jitter_buffer.buffer;
		jitter_buffer.buffer.clear();

		result.use_extrapolated_cosmos = true;

		struct step_to_simulate {
			bool resubstantiate = false;
			guid_mapped_entropy entropy;
		};

		std::vector<step_to_simulate> entropies_to_simulate;

		bool reconciliate_predicted = false;

		for (size_t i = 0; i < new_commands.size(); ++i) {
			auto& new_command = new_commands[i];

			if (new_command.step_type == step_packaged_for_network::type::NEW_ENTROPY_WITH_HEARTBEAT) {
				cosmic_delta::decode(last_delta_unpacked, new_command.delta);
				referential_cosmos = last_delta_unpacked;

				entropies_to_simulate.clear();
				reconciliate_predicted = true;
			}
			else
				ensure(new_command.step_type == step_packaged_for_network::type::NEW_ENTROPY);

			step_to_simulate sim;
			sim.resubstantiate = new_command.shall_resubstantiate;
			sim.entropy = new_command.entropy;

			entropies_to_simulate.emplace_back(sim);

			const auto& actual_server_step = sim.entropy;
			const auto& predicted_server_step = predicted_steps.front();
			
			if (actual_server_step != predicted_server_step) {
				reconciliate_predicted = true;
			}

			if (sim.resubstantiate) {
				reconciliate_predicted = true;
			}

			if (new_command.next_client_commands_accepted) {
				ensure(predicted_steps.size() > 0);
				predicted_steps.erase(predicted_steps.begin());
			}
		}

		for (const auto& e : entropies_to_simulate) {
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
			const auto potential_mismatches = acquire_potential_mismatches(predicted_cosmos);

			predicted_cosmos = referential_cosmos;

			for (const auto& s : predicted_steps) {
				advance(cosmic_entropy(s, predicted_cosmos), predicted_cosmos);
			}

			drag_mismatches_into_past(predicted_cosmos, potential_mismatches);
		}

		return std::move(result);
	}
};