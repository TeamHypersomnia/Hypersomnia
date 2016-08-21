#include "simulation_exchange.h"
#include "cosmic_delta.h"

#include "game/transcendental/types_specification/all_component_includes.h"

void simulation_receiver::acquire_new_entropy(const cosmic_entropy& entropy) {
	command new_command;
	new_command.command_type = command::type::NEW_INPUT;
	new_command.entropy = entropy;

	jitter_buffer.acquire_new_command(std::move(new_command));
}

void simulation_receiver::acquire_new_entropy_with_heartbeat(const cosmic_entropy& entropy, const augs::stream& delta) {
	command new_command;
	new_command.command_type = command::type::NEW_HEARTBEAT;
	new_command.delta = delta;
	new_command.entropy = entropy;

	jitter_buffer.acquire_new_command(std::move(new_command));
}

simulation_receiver::unpacked_steps simulation_receiver::unpack_deterministic_steps(cosmos& properly_stepped_cosmos, cosmos& extrapolated_cosmos, cosmos& last_delta_unpacked) {
	unpacked_steps result;

	auto new_commands = jitter_buffer.unpack_commands_once();

	bool unpacked_successfully = new_commands.size() > 0;

	if (unpacked_successfully) {
		result.use_extrapolated_cosmos = false;

		for (size_t i = 0; i < new_commands.size(); ++i) {
			if (new_commands[i].command_type == command::type::NEW_HEARTBEAT) {
				cosmic_delta::decode(last_delta_unpacked, new_commands[i].delta);
				properly_stepped_cosmos = last_delta_unpacked;

				result.steps_for_proper_cosmos.clear();
			}
			else
				ensure(new_commands[i].command_type == command::type::NEW_INPUT);
			
			result.steps_for_proper_cosmos.emplace_back(new_commands[i].entropy);
		}
	}
	else {
		if (jitter_buffer.get_steps_extrapolated() == 1)
			extrapolated_cosmos = properly_stepped_cosmos;

		result.use_extrapolated_cosmos = true;
	}

	return result;
}


void simulation_broadcast::simulate(const input_context& context) {
}

void simulation_broadcast::set_delta_heartbeat_interval(const augs::fixed_delta& dt, float ms) {
	delta_heartbeat_interval_in_steps = static_cast<unsigned>(ms / dt.in_milliseconds());
}

