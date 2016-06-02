#include "trigger_detector_system.h"
#include "entity_system/world.h"

#include "../messages/intent_message.h"

#include "../messages/trigger_hit_confirmation_message.h"
#include "../messages/trigger_hit_request_message.h"
#include "../messages/collision_message.h"

#include "../messages/intent_message.h"

#include "../systems/physics_system.h"

#include "../globals/filters.h"

#include "../components/trigger_component.h"
#include "../components/physics_component.h"
#include "../components/transform_component.h"

#include "../components/trigger_collision_detector_component.h"
#include "../components/trigger_query_detector_component.h"

void trigger_detector_system::consume_trigger_detector_presses() {
	auto& trigger_presses = parent_world.get_message_queue<messages::intent_message>();

	for (auto& e : trigger_presses) {
		if (e.intent == intent_type::QUERY_TOUCHING_TRIGGERS) {
			auto* trigger_query_detector = e.subject->find<components::trigger_query_detector>();

			if (trigger_query_detector) {
				trigger_query_detector->detection_intent_enabled = e.pressed_flag;

				if (trigger_query_detector->spam_trigger_requests_when_detection_intented) {
					if (trigger_query_detector->detection_intent_enabled)
						e.subject->enable(trigger_query_detector);
					else
						e.subject->disable(trigger_query_detector);
				}
				else if(e.pressed_flag) {
					e.subject->disable(trigger_query_detector);

					messages::trigger_hit_request_message request;
					request.detector = e.subject;
					parent_world.post_message(request);
				}
			}
		}
		else if (e.intent == intent_type::DETECT_TRIGGER_COLLISIONS) {
			auto* trigger_collision_detector = e.subject->find<components::trigger_collision_detector>();

			if (trigger_collision_detector) {
				trigger_collision_detector->detection_intent_enabled = e.pressed_flag;
			}
		}
	}
}

void trigger_detector_system::post_trigger_requests_from_continuous_detectors() {
	auto targets_copy = targets;

	for (auto& t : targets_copy) {
		if (!t->get<components::trigger_query_detector>().detection_intent_enabled)
			t->disable<components::trigger_query_detector>();
		else {
			messages::trigger_hit_request_message request;
			request.detector = t;
			parent_world.post_message(request);
		}
	}
}

void trigger_detector_system::send_trigger_confirmations() {
	auto& confirmations = parent_world.get_message_queue<messages::trigger_hit_confirmation_message>();

	confirmations.clear();

	auto& collisions = parent_world.get_message_queue<messages::collision_message>();

	for (auto& c : collisions) {
		if (c.type != messages::collision_message::event_type::PRE_SOLVE)
			continue;

		auto* collision_detector = c.subject->find<components::trigger_collision_detector>();
		auto* trigger = c.collider->find<components::trigger>();

		if (collision_detector && trigger && trigger->react_to_collision_detectors && collision_detector->detection_intent_enabled
			) {
			messages::trigger_hit_confirmation_message confirmation;
			confirmation.detector_body = c.subject;
			confirmation.trigger = c.collider;
			parent_world.post_message(confirmation);
		}
	}

	auto& requests = parent_world.get_message_queue<messages::trigger_hit_request_message>();

	for (auto& e : requests) {
		auto& trigger_query_detector = e.detector->get<components::trigger_query_detector>();
		auto& detector_body = e.detector;
		
		std::vector<augs::entity_id> found_triggers;

		auto found_physical_triggers = parent_world.get_system<physics_system>().query_body(detector_body, filters::trigger());

		for (auto found_trigger : found_physical_triggers.entities) {
			auto* maybe_trigger = found_trigger->find<components::trigger>();
			
			if (maybe_trigger && maybe_trigger->react_to_query_detectors)
				found_triggers.push_back(found_trigger);
		}

		for (auto& t : found_triggers) {
			messages::trigger_hit_confirmation_message confirmation;
			confirmation.trigger = t;
			confirmation.detector_body = detector_body;
			trigger_query_detector.detection_intent_enabled = false;
			parent_world.post_message(confirmation);
			break;
		}
	}

	requests.clear();
}
