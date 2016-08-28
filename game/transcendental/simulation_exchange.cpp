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

void simulation_exchange::write_entropy(augs::stream& output, const packaged_step& written) {
	ensure(written.entropy.entropy_per_entity.size() < std::numeric_limits<unsigned char>::max());

	augs::write_object(output, written.shall_resubstantiate);

	unsigned char num_entropied_entities = written.entropy.entropy_per_entity.size();
	augs::write_object(output, num_entropied_entities);

	for (const auto& per_entity : written.entropy.entropy_per_entity) {
		augs::write_object(output, per_entity.first);
		augs::write_vector_of_objects(output, per_entity.second);
	}
}

simulation_exchange::packaged_step simulation_exchange::read_entropy(augs::stream& in) {
	packaged_step new_command;
	auto& new_entropy = new_command.entropy;
	
	augs::read_object(in, new_command.shall_resubstantiate);

	unsigned char num_entropied_entities = 0;
	augs::read_object(in, num_entropied_entities);

	while (num_entropied_entities--) {
		unsigned guid;
		augs::read_object(in, guid);

		auto& new_entity_entropy = new_entropy.entropy_per_entity[guid];

		ensure(new_entity_entropy.empty());

		augs::read_vector_of_objects(in, new_entity_entropy);
	}

	return std::move(new_command);
}

void simulation_exchange::write_packaged_step_to_stream(augs::stream& output, const packaged_step& written) {
	if (written.step_type == packaged_step::type::NEW_ENTROPY) {
		augs::write_object(output, network_command::ENTROPY_FOR_NEXT_STEP);

		write_entropy(output, written);

	}
	else if (written.step_type == packaged_step::type::NEW_ENTROPY_WITH_HEARTBEAT) {
		augs::write_object(output, network_command::ENTROPY_WITH_HEARTBEAT_FOR_NEXT_STEP);

		write_entropy(output, written);

		augs::write_sized_stream(output, written.delta);
	}
	else
		ensure(false);
}

simulation_exchange::packaged_step simulation_exchange::read_entropy_for_next_step(augs::stream& in) {
	network_command command_type;
	augs::read_object(in, command_type);

	ensure(command_type == network_command::ENTROPY_FOR_NEXT_STEP);
	
	auto new_command = read_entropy(in);
	new_command.step_type = packaged_step::type::NEW_ENTROPY;

	return std::move(new_command);
}

simulation_exchange::packaged_step simulation_exchange::read_entropy_with_heartbeat_for_next_step(augs::stream& in) {
	network_command command_type;
	augs::read_object(in, command_type);

	ensure(command_type == network_command::ENTROPY_WITH_HEARTBEAT_FOR_NEXT_STEP);

	auto new_command = read_entropy(in);
	new_command.step_type = packaged_step::type::NEW_ENTROPY_WITH_HEARTBEAT;
	augs::read_sized_stream(in, new_command.delta);

	return std::move(new_command);
}