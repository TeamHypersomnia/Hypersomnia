#pragma once
#include "entity_system/processing_system.h"
#include "../components/input_receiver_component.h"
#include "../messages/intent_message.h"

#include "../messages/crosshair_intent_message.h"
#include "../messages/raw_window_input_message.h"

#include "window_framework/event.h"
#include "misc/step_player.h"

using namespace augs;

struct input_system : public processing_system_templated<components::input_receiver> {
	struct context {
		std::unordered_map<window::event::keys::key, std::vector<messages::intent_message::intent_type>> key_to_intent;
		std::unordered_map<window::event::message,	 std::vector<messages::intent_message::intent_type>> event_to_intent;
		bool enabled;
		context();

		void map_key_to_intent(window::event::keys::key, messages::intent_message::intent_type);
		void map_event_to_intent(window::event::message, messages::intent_message::intent_type);
	};

	void acquire_new_events_posted_by_drawing_time_systems();
	void acquire_new_raw_window_inputs();
	void post_input_intents_from_new_raw_window_inputs();

	void post_input_intents_from_all_raw_window_inputs_since_last_step();
	void post_all_events_posted_by_drawing_time_systems_since_last_step();

	void replay_drawing_time_events_passed_to_last_logic_step();


	void add_context(context);
	void clear_contexts();

	std::vector<context> active_contexts;

	input_system::input_system(world& parent_world);

	template <class event_type>
	struct event_unpacker_and_recorder {
		world& parent_world;
		event_unpacker_and_recorder(world& parent_world) : parent_world(parent_world) {}

		struct events_per_step {
			std::vector<event_type> events;

			void serialize(std::ofstream& f) {
				augs::serialize_vector(f, events);
			}

			bool should_serialize() {
				return !events.empty();
			}

			void deserialize(std::ifstream& f) {
				augs::deserialize_vector(f, events);
			}
		};

		step_player<events_per_step> player;

		events_per_step inputs_from_last_step;

		events_per_step buffered_inputs_for_next_step;
		events_per_step inputs_from_last_drawing_time;

		void acquire_new_events_posted_by_drawing_time_systems() {
			if (player.is_replaying()) {
				parent_world.get_message_queue<event_type>().clear();
				return;
			}

			inputs_from_last_drawing_time.events = parent_world.get_message_queue<event_type>();

			for (auto& m : inputs_from_last_drawing_time.events)
				buffered_inputs_for_next_step.events.push_back(m);

			parent_world.get_message_queue<event_type>().clear();
		}
		
		void pass_last_unpacked_logic_events_for_drawing_time_approximation() {
			if (player.is_replaying()) {
				parent_world.get_message_queue<event_type>() = inputs_from_last_step.events;
				// we do it only once per step
				inputs_from_last_step.events.clear();
			}
		}

		void clear_step() {
			buffered_inputs_for_next_step.events.clear();
		}

		void biserialize() {
			player.biserialize(buffered_inputs_for_next_step);
		}

		void generate_events_for_logic_step() {
			biserialize();
			parent_world.get_message_queue<event_type>() = buffered_inputs_for_next_step.events;
			inputs_from_last_step = buffered_inputs_for_next_step;
			clear_step();
		}
	};


	event_unpacker_and_recorder<messages::raw_window_input_message> raw_window_input_player;
	event_unpacker_and_recorder<messages::crosshair_intent_message> crosshair_intent_player;

private:
	void post_intents_from_inputs(const std::vector<messages::raw_window_input_message>&);
};