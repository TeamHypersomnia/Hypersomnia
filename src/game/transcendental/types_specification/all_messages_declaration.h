#pragma once
#include "game/detail/inventory/item_slot_transfer_request_declaration.h"

namespace augs {
	template <class...>
	class storage_for_message_queues;
}

namespace messages {
	struct intent_message;
	struct interpolation_correction_request;
	struct damage_message;
	struct queue_destruction;
	struct will_soon_be_deleted;
	struct animation_message;
	struct movement_event;
	struct collision_message;
	struct gunshot_response;
	struct crosshair_intent_message;
	struct melee_swing_response;
	struct health_event;
	struct visibility_information_request;
	struct visibility_information_response;
	struct line_of_sight_request;
	struct line_of_sight_response;
	struct item_picked_up_message;
	struct exhausted_cast;
}

struct exploding_ring_input;
struct thunder_input;

typedef augs::storage_for_message_queues <
	messages::intent_message,
	messages::interpolation_correction_request,
	messages::damage_message,
	messages::queue_destruction,
	messages::will_soon_be_deleted,
	messages::animation_message,
	messages::movement_event,
	messages::collision_message,
	messages::gunshot_response,
	messages::crosshair_intent_message,
	messages::melee_swing_response,
	messages::health_event,
	messages::visibility_information_request,
	messages::visibility_information_response,
	messages::line_of_sight_request,
	messages::line_of_sight_response,
	messages::item_picked_up_message,
	messages::exhausted_cast,
	exploding_ring_input,
	thunder_input,
	item_slot_transfer_request
> storage_for_all_message_queues;