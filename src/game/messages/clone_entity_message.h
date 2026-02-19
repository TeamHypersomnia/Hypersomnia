#pragma once
#include <functional>
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/step_declaration.h"

namespace messages {
	/*
	 * A simple, untyped clone entity message used by the destruction system.
	 * The callback receives the newly cloned entity handle and should set up
	 * the clone's state (position, health, etc.). The callback captures all 
	 * required data by value to avoid any dangling references.
	 */
	struct clone_entity_message {
		entity_id source;
		std::function<void(entity_handle, logic_step)> post_clone = nullptr;
	};
}
