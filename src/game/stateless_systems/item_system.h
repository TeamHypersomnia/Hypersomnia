#pragma once
#include "game/cosmos/step_declaration.h"

class item_system {
public:
	void handle_throw_item_intents(const logic_step step);

	void handle_reload_intents(const logic_step step);
	void advance_reloading_contexts(const logic_step step);

	void handle_wielding_requests(const logic_step step);
};