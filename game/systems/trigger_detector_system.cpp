#include "trigger_detector_system.h"
#include "game/cosmos.h"

#include "game/messages/intent_message.h"

#include "game/messages/trigger_hit_confirmation_message.h"
#include "game/messages/trigger_hit_request_message.h"
#include "game/messages/collision_message.h"

#include "game/messages/intent_message.h"

#include "game/systems/physics_system.h"

#include "game/globals/filters.h"

#include "game/components/trigger_component.h"
#include "game/components/physics_component.h"
#include "game/components/transform_component.h"

#include "game/components/trigger_collision_detector_component.h"
#include "game/components/trigger_query_detector_component.h"
#include "game/entity_handle.h"
#include "game/step_state.h"

void trigger_detector_system::consume_trigger_detector_presses() {
	auto& trigger_presses = step.messages.get_queue<messages::intent_message>();

	for (auto& e : trigger_presses) {
		if (e.intent == intent_type::QUERY_TOUCHING_TRIGGERS) {
			auto* trigger_query_detector = e.subject.find<components::trigger_query_detector>();

			if (trigger_query_detector) {
				trigger_query_detector->detection_intent_enabled = e.pressed_flag;

				if (trigger_query_detector->spam_trigger_requests_when_detection_intented) {
					if (trigger_query_detector->detection_intent_enabled)
						e.subject.unskip_processing_in(processing_subjects::trigger_query_detector>();
					else
						e.subject.skip_processing_in(processing_subjects::trigger_query_detector>();
				}
				else if(e.pressed_flag) {
					e.subject.deactivate(trigger_query_detector);

					messages::trigger_hit_request_message request;
					request.detector = e.subject;
					step.messages.post(request);
				}
			}
		}
		else if (e.intent == intent_type::DETECT_TRIGGER_COLLISIONS) {
			auto* trigger_collision_detector = e.subject.find<components::trigger_collision_detector>();

			if (trigger_collision_detector) {
				trigger_collision_detector->detection_intent_enabled = e.pressed_flag;
			}
		}
	}
}

void trigger_detector_system::post_trigger_requests_from_continuous_detectors(cosmos& cosmos, step_state& step) {
	auto targets_copy = cosmos.get(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);

	for (auto& t : targets_copy) {
		if (!t.get<components::trigger_query_detector>().detection_intent_enabled)
			t.skip_processing_in(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);
		else {
			messages::trigger_hit_request_message request;
			request.detector = t;
			step.messages.post(request);
		}
	}
}

void trigger_detector_system::send_trigger_confirmations() {
	auto& confirmations = step.messages.get_queue<messages::trigger_hit_confirmation_message>();

	confirmations.clear();

	auto& collisions = step.messages.get_queue<messages::collision_message>();

	for (auto& c : collisions) {
		if (c.type != messages::collision_message::event_type::PRE_SOLVE)
			continue;

		auto* collision_detector = c.subject.find<components::trigger_collision_detector>();
		auto* trigger = c.collider.find<components::trigger>();

		if (collision_detector && trigger && trigger->react_to_collision_detectors && collision_detector->detection_intent_enabled
			) {
			messages::trigger_hit_confirmation_message confirmation;
			confirmation.detector_body = c.subject;
			confirmation.trigger = c.collider;
			step.messages.post(confirmation);
		}
	}

	auto& requests = step.messages.get_queue<messages::trigger_hit_request_message>();

	for (auto& e : requests) {
		auto& trigger_query_detector = e.detector.get<components::trigger_query_detector>();
		auto& detector_body = e.detector;
		
		std::vector<entity_id> found_triggers;

		auto found_physical_triggers = parent_cosmos.stateful_systems.get<physics_system>().query_body(detector_body, filters::trigger());

		for (auto found_trigger : found_physical_triggers.entities) {
			auto* maybe_trigger = found_trigger.find<components::trigger>();
			
			if (maybe_trigger && maybe_trigger->react_to_query_detectors)
				found_triggers.push_back(found_trigger);
		}

		for (auto& t : found_triggers) {
			messages::trigger_hit_confirmation_message confirmation;
			confirmation.trigger = t;
			confirmation.detector_body = detector_body;
			trigger_query_detector.detection_intent_enabled = false;
			step.messages.post(confirmation);
			break;
		}
	}

	requests.clear();
}
