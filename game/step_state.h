#pragma once
#include "game/types_specification/all_messages_declaration.h"

class step_state {
public:
	storage_for_all_message_queues messages;

	step_state();
};