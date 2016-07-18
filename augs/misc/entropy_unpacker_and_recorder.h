#pragma once
#include <vector>
#include "augs/templates.h"
#include "step_player.h"

namespace augs {
	template <class event_type>
	struct entropy_unpacker_and_recorder {
		typedef std::vector<event_type> events_per_step;
		
		step_player<events_per_step> player;

		events_per_step buffered_inputs_for_next_step;
		events_per_step inputs_from_last_drawing_time;

		void buffer_new_events_before_step(std::vector<event_type> events) {
			if (player.is_replaying()) {
				step.messages.get_queue<event_type>().clear();
				return;
			}

			inputs_from_last_drawing_time.events = step.messages.get_queue<event_type>();

			for (auto& m : events)
				buffered_inputs_for_next_step.events.push_back(m);
		}

		void biserialize() {
			player.biserialize(buffered_inputs_for_next_step);
		}

		std::vector<event_type> get_pending_inputs_for_logic() {
			auto res = buffered_inputs_for_next_step.events;
			auto& world_msgs = step.messages.get_queue<event_type>();
			res.insert(res.end(), world_msgs.begin(), world_msgs.end());
			return res;
		}

		void generate_events_for_logic_step() {
			biserialize();
			step.messages.get_queue<event_type>() = buffered_inputs_for_next_step.events;
			buffered_inputs_for_next_step.events.clear();
		}
	};
}