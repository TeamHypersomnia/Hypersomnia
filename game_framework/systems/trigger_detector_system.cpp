#include "trigger_detector_system.h"
#include "entity_system/world.h"

#include "../messages/intent_message.h"

#include "../messages/trigger_hit_confirmation_message.h"
#include "../messages/trigger_hit_request_message.h"

#include "../messages/intent_message.h"

#include "../systems/physics_system.h"

#include "../globals/filters.h"

#include "../components/trigger_component.h"
#include "../components/physics_component.h"
#include "../components/transform_component.h"
#include "../components/trigger_detector_component.h"

void trigger_detector_system::find_trigger_collisions_and_send_confirmations() {
	auto& intents = parent_world.get_message_queue<messages::intent_message>();
	auto& requests = parent_world.get_message_queue<messages::trigger_hit_request_message>();
	auto& confirmations = parent_world.get_message_queue<messages::trigger_hit_confirmation_message>();

	for (auto& e : intents) {
		if (e.intent == messages::intent_message::PRESS_TRIGGER && e.pressed_flag) {
			auto* maybe_detector = e.subject->find<components::trigger_detector>();

			if (maybe_detector) {
				messages::trigger_hit_request_message request;
				request.detector = e.subject;
				request.pressed_flag = e.pressed_flag;
				parent_world.post_message(request);
			}
		}
	}

	confirmations.clear();

	for (auto& e : requests) {
		auto& trigger_detector = e.detector->get<components::trigger_detector>();

		std::vector<augs::entity_id> found_triggers;

		if (trigger_detector.use_physics_of_detector) {
			auto found_physical_triggers = parent_world.get_system<physics_system>().query_body(e.detector, filters::trigger());

			for (auto& t : found_physical_triggers.bodies) {
				auto found_trigger = t->GetUserData();
				auto* maybe_trigger = found_trigger->find<components::trigger>();
				
				if (maybe_trigger)
					found_triggers.push_back(found_trigger);
			}
		}
		else {

		}

		for (auto& t : found_triggers) {
			messages::trigger_hit_confirmation_message confirmation;
			confirmation.trigger = t;
			confirmation.detector = e.detector;
			parent_world.post_message(confirmation);
			break;
		}
	}

	requests.clear();
}
