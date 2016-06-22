#pragma once
#include "game/components/input_receiver_component.h"
#include "game/messages/intent_message.h"

#include "game/messages/crosshair_intent_message.h"
#include "game/messages/raw_window_input_message.h"
#include "game/messages/gui_intents.h"

#include "window_framework/event.h"
#include "misc/step_player.h"

struct input_system {



	event_unpacker_and_recorder<messages::crosshair_intent_message> crosshair_intent_player;
	event_unpacker_and_recorder<messages::unmapped_intent_message> unmapped_intent_player;
	event_unpacker_and_recorder<messages::gui_item_transfer_intent> gui_item_transfer_intent_player;
	
	input_system::input_system(cosmos& parent_cosmos);

	void post_unmapped_intents_from_raw_window_inputs();
	void map_unmapped_intents_to_entities();
	void acquire_new_events_posted_by_drawing_time_systems();

	void post_all_events_posted_by_drawing_time_systems_since_last_step();

	void replay_found_recording();
	void record_and_save_this_session();

	bool found_recording() const;
	bool is_replaying() const;
};