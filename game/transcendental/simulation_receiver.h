#pragma once
#include "simulation_exchange.h"

class simulation_receiver : public simulation_exchange {
public:
	augs::jitter_buffer<packaged_step> jitter_buffer;

	class unpacked_steps {
		friend class simulation_receiver;
		std::vector<guid_mapped_entropy> steps_for_proper_cosmos;
	public:
		bool use_extrapolated_cosmos = true;

		bool has_next_entropy() const;
		cosmic_entropy unpack_next_entropy(const cosmos& guid_mapper);
	};

	void read_entropy_for_next_step(augs::stream&, bool skip_command);
	void read_entropy_with_heartbeat_for_next_step(augs::stream&, bool skip_command);

	struct unpacking_result {
		bool use_extrapolated_cosmos = true;
	};

	template<class Step>
	unpacking_result unpack_deterministic_steps(cosmos& properly_stepped_cosmos, cosmos& last_delta_unpacked, Step pred) {
		unpacking_result result;

		auto new_commands = jitter_buffer.buffer;
		jitter_buffer.buffer.clear();

		result.use_extrapolated_cosmos = false;

		std::vector<guid_mapped_entropy> entropies_to_simulate;

		for (size_t i = 0; i < new_commands.size(); ++i) {
			if (new_commands[i].step_type == packaged_step::type::NEW_ENTROPY_WITH_HEARTBEAT) {
				cosmic_delta::decode(last_delta_unpacked, new_commands[i].delta);
				properly_stepped_cosmos = last_delta_unpacked;

				entropies_to_simulate.clear();
			}
			else
				ensure(new_commands[i].step_type == packaged_step::type::NEW_ENTROPY);

			entropies_to_simulate.emplace_back(new_commands[i].entropy);
		}

		for (auto& e : entropies_to_simulate) {
			const auto cosmic_entropy_for_this_step = cosmic_entropy(e, properly_stepped_cosmos);
			pred(cosmic_entropy_for_this_step, properly_stepped_cosmos);
			renderer::get_current().clear_logic_lines();
		}

		return std::move(result);
	}
};