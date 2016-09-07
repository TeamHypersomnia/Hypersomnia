#pragma once
#include "simulation_exchange.h"

namespace augs {
	namespace network {
		class client;
	}
}

class simulation_receiver : public simulation_exchange {
public:
	augs::jitter_buffer<packaged_step> jitter_buffer;
	std::vector<guid_mapped_entropy> predicted_steps;

	void read_entropy_for_next_step(augs::stream&, bool skip_command);
	void read_entropy_with_heartbeat_for_next_step(augs::stream&, bool skip_command);

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
		client.send_pending_redundant();

		predicted_steps.push_back(guid_mapped);
		advance(new_entropy, predicted_cosmos);
	}

	template<class Step>
	unpacking_result unpack_deterministic_steps(cosmos& referential_cosmos, cosmos& last_delta_unpacked, cosmos& predicted_cosmos, Step advance) {
		unpacking_result result;

		auto new_commands = jitter_buffer.buffer;
		jitter_buffer.buffer.clear();

		result.use_extrapolated_cosmos = true;

		std::vector<guid_mapped_entropy> entropies_to_simulate;

		for (size_t i = 0; i < new_commands.size(); ++i) {
			auto& new_command = new_commands[i];

			if (new_command.step_type == packaged_step::type::NEW_ENTROPY_WITH_HEARTBEAT) {
				cosmic_delta::decode(last_delta_unpacked, new_command.delta);
				referential_cosmos = last_delta_unpacked;

				entropies_to_simulate.clear();
			}
			else
				ensure(new_command.step_type == packaged_step::type::NEW_ENTROPY);

			entropies_to_simulate.emplace_back(new_command.entropy);

			if (new_command.next_client_commands_accepted) {
				ensure(predicted_steps.size() > 0);

				predicted_steps.erase(predicted_steps.begin());
			}
		}

		const unsigned previous_step = referential_cosmos.get_total_steps_passed();

		for (const auto& e : entropies_to_simulate) {
			const cosmic_entropy cosmic_entropy_for_this_step(e, referential_cosmos);
			advance(cosmic_entropy_for_this_step, referential_cosmos);
		}
		
		LOG("Unpacking from %x to %x", previous_step, referential_cosmos.get_total_steps_passed());

		const bool reconciliate_predicted = entropies_to_simulate.size() > 0;

		if (reconciliate_predicted) {
			unsigned predicted_was_at_step = predicted_cosmos.get_total_steps_passed();

			predicted_cosmos = referential_cosmos;

			for (const auto& s : predicted_steps) {
				advance(cosmic_entropy(s, predicted_cosmos), predicted_cosmos);
			}

			//ensure(predicted_cosmos.get_total_steps_passed() == predicted_was_at_step);

			//while (predicted_cosmos.get_total_steps_passed() < predicted_was_at_step) {
			//
			//}
		}

		return std::move(result);
	}
};