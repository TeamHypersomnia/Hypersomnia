#include "trigger_detector_system.h"
#include "game/transcendental/cosmos.h"

#include "game/messages/intent_message.h"

#include "game/messages/trigger_hit_confirmation_message.h"
#include "game/messages/trigger_hit_request_message.h"
#include "game/messages/collision_message.h"

#include "game/messages/intent_message.h"

#include "game/systems_temporary/physics_system.h"

#include "game/enums/filters.h"

#include "game/components/trigger_component.h"
#include "game/components/physics_component.h"
#include "game/components/transform_component.h"

#include "game/components/trigger_collision_detector_component.h"
#include "game/components/trigger_query_detector_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"

void trigger_detector_system::consume_trigger_detector_presses(fixed_step& step) const {
	auto& trigger_presses = step.messages.get_queue<messages::intent_message>();
	auto& cosmos = step.cosm;

	for (auto& e : trigger_presses) {
		auto subject = cosmos[e.subject];

		if (e.intent == intent_type::QUERY_TOUCHING_TRIGGERS) {
			auto* trigger_query_detector = subject.find<components::trigger_query_detector>();

			if (trigger_query_detector) {
				bool pressed = e.pressed_flag;

				trigger_query_detector->detection_intent_enabled = pressed;

				if (trigger_query_detector->spam_trigger_requests_when_detection_intented) {
					if (pressed)
						subject.get<components::processing>().enable_in(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);
					else
						subject.get<components::processing>().disable_in(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);
				}
				else if(pressed) {
					subject.get<components::processing>().disable_in(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);

					messages::trigger_hit_request_message request;
					request.detector = e.subject;
					step.messages.post(request);
				}
			}
		}
		else if (e.intent == intent_type::DETECT_TRIGGER_COLLISIONS) {
			auto* trigger_collision_detector = subject.find<components::trigger_collision_detector>();

			if (trigger_collision_detector) {
				trigger_collision_detector->detection_intent_enabled = e.pressed_flag;
			}
		}
	}
}

void trigger_detector_system::post_trigger_requests_from_continuous_detectors(fixed_step& step) const {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto targets_copy = cosmos.get(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);

	for (auto& t : targets_copy) {
		if (!t.get<components::trigger_query_detector>().detection_intent_enabled)
			t.get<components::processing>().disable_in(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);
		else {
			messages::trigger_hit_request_message request;
			request.detector = t;
			step.messages.post(request);
		}
	}
}

void trigger_detector_system::send_trigger_confirmations(fixed_step& step) const {
	auto& confirmations = step.messages.get_queue<messages::trigger_hit_confirmation_message>();
	auto& cosmos = step.cosm;

	confirmations.clear();

	auto& collisions = step.messages.get_queue<messages::collision_message>();

	for (auto& c : collisions) {
		if (c.type != messages::collision_message::event_type::PRE_SOLVE)
			continue;

		auto* collision_detector = cosmos[c.subject].find<components::trigger_collision_detector>();
		auto* trigger = cosmos[c.collider].find<components::trigger>();

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
		auto& trigger_query_detector = cosmos[e.detector].get<components::trigger_query_detector>();
		auto detector_body = cosmos[e.detector];
		
		std::vector<entity_id> found_triggers;

		auto found_physical_triggers = cosmos.systems_temporary.get<physics_system>().query_body(detector_body, filters::trigger());

		for (auto found_trigger_id : found_physical_triggers.entities) {
			auto found_trigger = cosmos[found_trigger_id];
			auto* maybe_trigger = found_trigger.find<components::trigger>();
			
			if (maybe_trigger && maybe_trigger->react_to_query_detectors)
				found_triggers.push_back(found_trigger);
		}

		for (auto t : found_triggers) {
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
