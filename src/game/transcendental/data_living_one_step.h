#pragma once
#include <unordered_map>

#include "game/transcendental/types_specification/all_messages_declaration.h"
#include "game/messages/visibility_information.h"
#include "augs/entity_system/storage_for_message_queues.h"

typedef std::unordered_map<entity_id, messages::visibility_information_response> calculated_visibility_map;
typedef std::unordered_map<entity_id, messages::line_of_sight_response> calculated_line_of_sight_map;

struct data_living_one_step {
	storage_for_all_message_queues messages;
	calculated_visibility_map calculated_visibility;
	calculated_line_of_sight_map calculated_line_of_sight;

	void clear_all();
};