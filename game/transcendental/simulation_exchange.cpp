#include "augs/misc/templated_readwrite.h"

#include "simulation_exchange.h"
#include "cosmic_delta.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/network_commands.h"
#include "game/transcendental/cosmos.h"

//void simulation_receiver::read_commands_from_stream(augs::stream& in) {
//	auto new_commands = simulation_exchange::read_commands_from_stream(in);
//	jitter_buffer.acquire_new_commands(new_commands.begin(), new_commands.end());
//}

void simulation_exchange::write_packaged_step_to_stream(augs::stream& output, const packaged_step& written) {
	if (written.step_type == packaged_step::type::NEW_ENTROPY) {
		augs::write_object(output, network_command::ENTROPY_FOR_NEXT_STEP);
		augs::write_object(output, written.shall_resubstantiate);
		augs::write_object(output, written.next_client_commands_accepted);
		augs::write_object(output, written.entropy);

	}
	else if (written.step_type == packaged_step::type::NEW_ENTROPY_WITH_HEARTBEAT) {
		augs::write_object(output, network_command::ENTROPY_WITH_HEARTBEAT_FOR_NEXT_STEP);
		augs::write_object(output, written.next_client_commands_accepted);
		augs::write_object(output, written.entropy);

		augs::write_sized_stream(output, written.delta);
	}
	else
		ensure(false);
}

simulation_exchange::packaged_step simulation_exchange::read_entropy_for_next_step(augs::stream& in) {
	network_command command_type;
	augs::read_object(in, command_type);

	ensure(command_type == network_command::ENTROPY_FOR_NEXT_STEP);
	
	packaged_step new_command;
	new_command.step_type = packaged_step::type::NEW_ENTROPY;

	augs::read_object(in, new_command.shall_resubstantiate);
	augs::read_object(in, new_command.next_client_commands_accepted);
	augs::read_object(in, new_command.entropy);

	return std::move(new_command);
}

simulation_exchange::packaged_step simulation_exchange::read_entropy_with_heartbeat_for_next_step(augs::stream& in) {
	network_command command_type;
	augs::read_object(in, command_type);

	ensure(command_type == network_command::ENTROPY_WITH_HEARTBEAT_FOR_NEXT_STEP);

	packaged_step new_command;
	new_command.step_type = packaged_step::type::NEW_ENTROPY_WITH_HEARTBEAT;

	augs::read_object(in, new_command.next_client_commands_accepted);
	augs::read_object(in, new_command.entropy);
	augs::read_sized_stream(in, new_command.delta);

	return std::move(new_command);
}