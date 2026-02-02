#pragma once
#include <functional>
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/entity_flavour_id.h"
#include "game/cosmos/step_declaration.h"

namespace messages {
	/*
	 * An untyped create entity message that uses entity_flavour_id (not typed).
	 * Used for spawning entities where the exact type is not known at compile time,
	 * such as coin spawning from destructible objects.
	 */
	struct just_create_entity_message {
		entity_flavour_id flavour;
		std::function<void(entity_handle, logic_step)> post_create = nullptr;
	};
}
