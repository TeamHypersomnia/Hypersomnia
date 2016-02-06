#pragma once
#include "entity_system/entity_id.h"
#include "../components/container_component.h"

namespace messages {
	struct item_slot_transfer_request {
		float milliseconds_passed = 0.f;
		float calculated_transfer_duration_ms = 0.f;

		bool has_transfer_duration_been_calculated = false;
		bool delete_this_message = false;

		augs::entity_id item;
		components::container::slot_id target_slot;
	};
}