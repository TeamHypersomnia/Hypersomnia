#pragma once
#include <unordered_map>

#include "game/organization/all_messages_declaration.h"
#include "game/messages/visibility_information.h"
#include "augs/entity_system/storage_for_message_queues.h"

using calculated_visibility_map = std::unordered_map<entity_id, messages::visibility_information_response>;

struct data_living_one_step {
	all_message_queues messages;
	calculated_visibility_map calculated_visibility;

	void clear();
};