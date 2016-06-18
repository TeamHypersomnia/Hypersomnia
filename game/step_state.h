#pragma once
#include "game/types_specification/message_queues_instantiation.h"

class step_state {
public:
	storage_for_all_message_queues messages;

	step_state();
};