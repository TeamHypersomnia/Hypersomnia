#pragma once
#include "game/types_specification/all_messages_declaration.h"
#include "misc/delta.h"
class cosmos;

class variable_step {
public:
	storage_for_all_message_queues messages;
	
	const cosmos& cosm;
	augs::variable_delta delta;

	augs::variable_delta get_delta() const;
};

class fixed_step {
public:
	storage_for_all_message_queues messages;
	cosmos& cosm;

	augs::fixed_delta get_delta() const;
};