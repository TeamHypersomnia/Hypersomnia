#pragma once
namespace augs {
	template <class...>
	class storage_for_message_queues;
}

namespace messages {
	struct intent_message;
	struct damage_message;
	struct queue_destruction;
	struct will_soon_be_deleted;
	struct animation_message;
	struct movement_response;
	struct collision_message;
	struct create_particle_effect;
	struct gunshot_response;
	struct raw_window_input_message;
	struct unmapped_intent_message;
	struct crosshair_intent_message;
	struct trigger_hit_confirmation_message;
	struct trigger_hit_request_message;
	struct new_entity_message;
	struct camera_render_request_message;
	struct item_slot_transfer_request;
	struct gui_item_transfer_intent;
	struct melee_swing_response;
	struct health_event;
}

typedef augs::storage_for_message_queues <
	messages::intent_message,
	messages::damage_message,
	messages::queue_destruction,
	messages::will_soon_be_deleted,
	messages::animation_message,
	messages::movement_response,
	messages::collision_message,
	messages::create_particle_effect,
	messages::gunshot_response,
	messages::raw_window_input_message,
	messages::unmapped_intent_message,
	messages::crosshair_intent_message,
	messages::trigger_hit_confirmation_message,
	messages::trigger_hit_request_message,
	messages::new_entity_message,
	messages::camera_render_request_message,
	messages::item_slot_transfer_request,
	messages::gui_item_transfer_intent,
	messages::melee_swing_response,
	messages::health_event
> storage_for_all_message_queues;