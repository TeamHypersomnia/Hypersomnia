#include "augs/misc/templated_readwrite.h"

#include "simulation_exchange.h"
#include "cosmic_delta.h"

#include "game/transcendental/types_specification/all_component_includes.h"

void simulation_receiver::read_commands_from_stream(augs::stream& in) {
	auto new_commands = simulation_exchange::read_commands_from_stream(in);
	jitter_buffer.acquire_new_commands(new_commands.begin(), new_commands.end());
}

std::vector<simulation_exchange::command> simulation_exchange::read_commands_from_stream(augs::stream& in) {
	std::vector<command> result;
	
	if (in.get_unread_bytes() == 0)
		return result;

	unsigned short num_commands = 0;
	augs::read_object(in, num_commands);

	while (num_commands--) {
		command new_command;
		auto& new_entropy = new_command.entropy;
		
		unsigned char num_entropied_entities = 0;
		augs::read_object(in, num_entropied_entities);

		while (num_entropied_entities--) {
			/*
			this can't be, because some entities may not yet exist!
			we must read guids and convert them to entity_ids at a later time.

			entity_id true_entity_id;

			{
			unsigned guid = 0;
			augs::read_object(in, guid);
			true_entity_id = guid_mapper.get_entity_by_guid(guid);
			}

			*/

			entity_id key;
			augs::read_object(in, key.guid);
			
			auto& new_entity_entropy = new_entropy.entropy_per_entity[key];
			
			ensure(new_entity_entropy.empty());

			augs::read_vector_of_objects(in, new_entity_entropy);
		}

		bool has_heartbeat = false;
		augs::read_object(in, has_heartbeat);

		if (has_heartbeat) {
			new_command.command_type = command::type::NEW_HEARTBEAT;

			auto& new_delta = new_command.delta;
			augs::read_sized_stream(in, new_delta);
		}
		else
			new_command.command_type = command::type::NEW_INPUT;

		result.emplace_back(std::move(new_command));
	}

	return std::move(result);
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

	return std::move(result);
}

void simulation_broadcast::set_delta_heartbeat_interval(const augs::fixed_delta& dt, float ms) {
	delta_heartbeat_interval_in_steps = static_cast<unsigned>(ms / dt.in_milliseconds());
}

