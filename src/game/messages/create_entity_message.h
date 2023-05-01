#pragma once
#include <vector>
#include <functional>
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/typed_entity_handle_declaration.h"
#include "game/messages/message.h"
#include "game/cosmos/step_declaration.h"

template <class E>
struct entity_solvable;

namespace messages {
	template <class E>
	struct create_entity_message {
		typed_entity_flavour_id<E> flavour_id;
		typed_entity_id<E> cloning_source;

		std::function<void(ref_typed_entity_handle<E>, entity_solvable<E>&)> pre_construction = nullptr;
		std::function<void(ref_typed_entity_handle<E>, logic_step)> post_construction = nullptr;
	};
}