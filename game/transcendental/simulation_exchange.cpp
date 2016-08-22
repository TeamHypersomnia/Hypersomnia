#include "augs/misc/templated_readwrite.h"

#include "simulation_exchange.h"
#include "cosmic_delta.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/network_commands.h"

//void simulation_receiver::read_commands_from_stream(augs::stream& in) {
//	auto new_commands = simulation_exchange::read_commands_from_stream(in);
//	jitter_buffer.acquire_new_commands(new_commands.begin(), new_commands.end());
//}

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

void simulation_receiver::read_entropy_for_next_step(augs::stream& in) {
	jitter_buffer.acquire_new_command(simulation_exchange::read_entropy_for_next_step(in));
}

void simulation_receiver::read_entropy_with_heartbeat_for_next_step(augs::stream& in) {
	jitter_buffer.acquire_new_command(simulation_exchange::read_entropy_with_heartbeat_for_next_step(in));
}

simulation_receiver::unpacked_steps simulation_receiver::unpack_deterministic_steps(cosmos& properly_stepped_cosmos, cosmos& extrapolated_cosmos, cosmos& last_delta_unpacked) {
	unpacked_steps result;

	auto new_commands = jitter_buffer.unpack_commands_once();

	bool unpacked_successfully = new_commands.size() > 0;

	if (unpacked_successfully) {
		result.use_extrapolated_cosmos = false;

		for (size_t i = 0; i < new_commands.size(); ++i) {
			if (new_commands[i].command_type == command::type::NEW_ENTROPY_WITH_HEARTBEAT) {
				cosmic_delta::decode(last_delta_unpacked, new_commands[i].delta);
				properly_stepped_cosmos = last_delta_unpacked;

				result.steps_for_proper_cosmos.clear();
			}
			else
				ensure(new_commands[i].command_type == command::type::NEW_ENTROPY);
			
			result.steps_for_proper_cosmos.emplace_back(new_commands[i].entropy);
		}
	}
	else {
		if (jitter_buffer.get_steps_extrapolated() == 1)
			extrapolated_cosmos = properly_stepped_cosmos;

		result.use_extrapolated_cosmos = true;
	}

	return std::move(result);
}

bool simulation_receiver::unpacked_steps::has_next_entropy() const {
	return steps_for_proper_cosmos.size() > 0;
}

cosmic_entropy simulation_receiver::unpacked_steps::unpack_next_entropy(const cosmos& guid_mapper) {
	ensure(has_next_entropy());

	cosmic_entropy result;

	std::vector<std::pair<entity_id, std::vector<entity_intent>>> invalids;
	
	for (const auto& entry : result.entropy_per_entity)
		invalids.push_back(entry); 

	result.entropy_per_entity.clear();

	for (auto i : invalids) {
		i.first = guid_mapper.get_entity_by_guid(i.first.guid);
		result.entropy_per_entity.emplace(std::move(i));
	}

	return std::move(result);
}

void simulation_broadcast::set_delta_heartbeat_interval(const augs::fixed_delta& dt, float ms) {
	delta_heartbeat_interval_in_steps = static_cast<unsigned>(ms / dt.in_milliseconds());
}

