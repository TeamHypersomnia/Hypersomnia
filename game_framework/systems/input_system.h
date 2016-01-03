#pragma once
#include "entity_system/processing_system.h"
#include "../components/input_component.h"
#include "../messages/intent_message.h"

#include "window_framework/event.h"
#include "misc/input_player.h"

using namespace augs;

struct input_system : public processing_system_templated<components::input> {
	struct context {
		std::unordered_map<window::event::keys::key, messages::intent_message::intent_type> key_to_intent;
		std::unordered_map<window::event::message, messages::intent_message::intent_type> event_to_intent;
		bool enabled;
		context();

		void map_key_to_intent(window::event::keys::key, messages::intent_message::intent_type);
		void map_event_to_intent(window::event::message, messages::intent_message::intent_type);
	};

	struct inputs_per_step {
		std::vector<augs::window::event::state> events;

		bool should_serialize();
		void serialize(std::ofstream&);
		void deserialize(std::ifstream&);
	};

	using processing_system_templated::processing_system_templated;

	void generate_input_intents_for_next_step();
	
	void add_context(context);
	void clear_contexts();

	void clear() override;

	std::vector<context> active_contexts;
	input_player<inputs_per_step> player;
};