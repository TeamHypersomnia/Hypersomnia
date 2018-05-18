#pragma once
#include "game/detail/inventory/item_slot_transfer_request_declaration.h"

namespace augs {
	template <class...>
	class storage_for_message_queues;
}

namespace messages {
	struct intent_message;
	struct motion_message;
	struct interpolation_correction_request;
	struct damage_message;
	struct queue_destruction;
	struct will_soon_be_deleted;
	struct collision_message;
	struct gunshot_response;
	struct crosshair_motion_message;
	struct melee_swing_response;
	struct health_event;
	struct visibility_information_request;
	struct visibility_information_response;
	struct line_of_sight_request;
	struct line_of_sight_response;
	struct item_picked_up_message;
	struct start_particle_effect;
	struct stop_particle_effect;
	struct start_sound_effect;
	struct stop_sound_effect;
	struct exhausted_cast;
}

struct exploding_ring_input;
struct thunder_input;

typedef augs::storage_for_message_queues <
	messages::intent_message,
	messages::motion_message,
	messages::interpolation_correction_request,
	messages::damage_message,
	messages::queue_destruction,
	messages::will_soon_be_deleted,
	messages::collision_message,
	messages::gunshot_response,
	messages::crosshair_motion_message,
	messages::melee_swing_response,
	messages::health_event,
	messages::visibility_information_request,
	messages::visibility_information_response,
	messages::line_of_sight_request,
	messages::line_of_sight_response,
	messages::item_picked_up_message,
	messages::exhausted_cast,
	messages::start_particle_effect,
	messages::stop_particle_effect,
	messages::start_sound_effect,
	messages::stop_sound_effect,
	exploding_ring_input,
	thunder_input,
	item_slot_transfer_request
> all_message_queues;
