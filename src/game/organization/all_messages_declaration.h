#pragma once
#include "game/detail/inventory/item_slot_transfer_request_declaration.h"
#include "game/organization/all_entity_types_declaration.h"

namespace augs {
	template <class...>
	class storage_for_message_queues;
}

namespace messages {
	struct intent_message;
	struct motion_message;
	struct interpolation_correction_request;
	struct damage_message;
	struct queue_deletion;
	struct will_soon_be_deleted;
	struct collision_message;
	struct gunshot_message;
	struct health_event;
	struct visibility_information_request;
	struct performed_transfer_message;
	struct start_particle_effect;
	struct stop_particle_effect;
	struct start_sound_effect;
	struct start_multi_sound_effect;
	struct stop_sound_effect;
	struct exhausted_cast;
	struct changed_identities_message;
	struct battle_event_message;
	struct thunder_effect;
	struct exploding_ring_effect;
	struct pure_color_highlight;
	struct mode_notification;
	struct game_notification;
	struct hud_message;
	struct duel_of_honor_message;
	struct match_summary_message;
	struct duel_interrupted_message;
	struct team_match_start_message;
	struct match_summary_ended;

	template <class T>
	struct create_entity_message;
}

#define MAKE_CREATE_ENTITY_MESSAGE(entity_type) messages::create_entity_message<entity_type>,

using all_message_queues = augs::storage_for_message_queues<
	messages::intent_message,
	messages::motion_message,
	messages::damage_message,
	messages::queue_deletion,
	messages::will_soon_be_deleted,
	messages::collision_message,
	messages::health_event,
	messages::visibility_information_request,
	item_slot_transfer_request,

	/* Intermediates whose purpose is to ultimately generate an effect. */
	messages::battle_event_message,
	messages::exhausted_cast,
	messages::gunshot_message,

	/* Improves integrity of audiovisual state */
	messages::performed_transfer_message,
	messages::changed_identities_message,
	messages::interpolation_correction_request,

	/* Purely effect messages. The only recipient is the audiovisual state. */
	messages::start_particle_effect,
	messages::stop_particle_effect,

	messages::start_sound_effect,
	messages::start_multi_sound_effect,
	messages::stop_sound_effect,

	messages::thunder_effect,
	messages::exploding_ring_effect,
	messages::pure_color_highlight,

	messages::mode_notification,
	messages::game_notification,
	messages::hud_message,

	messages::duel_of_honor_message,
	messages::duel_interrupted_message,
	messages::match_summary_message,

	messages::team_match_start_message,

	FOR_ALL_ENTITY_TYPES(MAKE_CREATE_ENTITY_MESSAGE)

	messages::match_summary_ended
>;
