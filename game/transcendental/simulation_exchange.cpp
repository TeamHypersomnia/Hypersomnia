#include "augs/misc/templated_readwrite.h"

#include "simulation_exchange.h"
#include "cosmic_delta.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/network_commands.h"

//void simulation_receiver::read_commands_from_stream(augs::stream& in) {
//	auto new_commands = simulation_exchange::read_commands_from_stream(in);
//	jitter_buffer.acquire_new_commands(new_commands.begin(), new_commands.end());
//}

void simulation_exchange::write_entropy(augs::stream& output, const command& written, const cosmos& guid_mapper) {
	ensure(written.entropy.entropy_per_entity.size() < std::numeric_limits<unsigned char>::max());

	unsigned char num_entropied_entities = written.entropy.entropy_per_entity.size();
	augs::write_object(output, num_entropied_entities);

	for (const auto& per_entity : written.entropy.entropy_per_entity) {
		unsigned guid = guid_mapper[per_entity.first].get_guid();

		augs::write_object(output, guid);

		augs::write_vector_of_objects(output, per_entity.second);
	}
}

void simulation_exchange::write_command_to_stream(augs::stream& output, const command& written, const cosmos& guid_mapper) {
	if (written.command_type == command::type::NEW_ENTROPY) {
		augs::write_object(output, static_cast<unsigned char>(network_command::ENTROPY_FOR_NEXT_STEP));

		write_entropy(output, written, guid_mapper);

	}
	else if (written.command_type == command::type::NEW_ENTROPY_WITH_HEARTBEAT) {
		augs::write_object(output, static_cast<unsigned char>(network_command::ENTROPY_WITH_HEARTBEAT_FOR_NEXT_STEP));

		write_entropy(output, written, guid_mapper);

		augs::write_sized_stream(output, written.delta);
	}
	else
		ensure(false);
}

simulation_exchange::command simulation_exchange::read_entropy(augs::stream& in) {
	command new_command;
	auto& new_entropy = new_command.entropy;

	unsigned char num_entropied_entities = 0;
	augs::read_object(in, num_entropied_entities);

	while (num_entropied_entities--) {
		entity_id key;
		augs::read_object(in, key.guid);

		auto& new_entity_entropy = new_entropy.entropy_per_entity[key];

		ensure(new_entity_entropy.empty());

		augs::read_vector_of_objects(in, new_entity_entropy);
	}

	return std::move(new_command);
}

simulation_exchange::command simulation_exchange::read_entropy_for_next_step(augs::stream& in) {
	unsigned char command_byte;
	augs::read_object(in, command_byte);

	ensure(command_byte == static_cast<unsigned char>(network_command::ENTROPY_FOR_NEXT_STEP));
	
	auto new_command = read_entropy(in);
	new_command.command_type = command::type::NEW_ENTROPY;

	return std::move(new_command);
}

simulation_exchange::command simulation_exchange::read_entropy_with_heartbeat_for_next_step(augs::stream& in) {
	unsigned char command_byte;
	augs::read_object(in, command_byte);

	ensure(command_byte == static_cast<unsigned char>(network_command::ENTROPY_WITH_HEARTBEAT_FOR_NEXT_STEP));

	auto new_command = read_entropy(in);
	new_command.command_type = command::type::NEW_ENTROPY_WITH_HEARTBEAT;
	augs::read_sized_stream(in, new_command.delta);

	return std::move(new_command);
}

