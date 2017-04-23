#include "trigger_detector_system.h"
#include "game/transcendental/cosmos.h"

#include "game/messages/intent_message.h"

#include "game/messages/trigger_hit_confirmation_message.h"
#include "game/messages/trigger_hit_request_message.h"
#include "game/messages/collision_message.h"

#include "game/messages/intent_message.h"

#include "game/systems_inferred/physics_system.h"

#include "game/enums/filters.h"

#include "game/components/trigger_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/transform_component.h"

#include "game/components/item_component.h"
#include "game/components/trigger_collision_detector_component.h"
#include "game/components/trigger_query_detector_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

void trigger_detector_system::consume_trigger_detector_presses(const logic_step step) const {
	const auto& trigger_presses = step.transient.messages.get_queue<messages::intent_message>();
	auto& cosmos = step.cosm;

	for (const auto& e : trigger_presses) {
		const auto subject = cosmos[e.subject];

		if (e.intent == intent_type::QUERY_TOUCHING_TRIGGERS) {
			auto* const trigger_query_detector = subject.find<components::trigger_query_detector>();

			if (trigger_query_detector) {
				const bool pressed = e.is_pressed;

				trigger_query_detector->detection_intent_enabled = pressed;

				if (trigger_query_detector->spam_trigger_requests_when_detection_intented) {
					if (pressed) {
						subject.get<components::processing>().enable_in(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);
					}
					else {
						subject.get<components::processing>().disable_in(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);
					}
				}
				else if(pressed) {
					subject.get<components::processing>().disable_in(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);

					messages::trigger_hit_request_message request;
					request.detector = e.subject;
					step.transient.messages.post(request);
				}
			}
		}
		else if (e.intent == intent_type::DETECT_TRIGGER_COLLISIONS) {
			auto* const trigger_collision_detector = subject.find<components::trigger_collision_detector>();

			if (trigger_collision_detector) {
				trigger_collision_detector->detection_intent_enabled = e.is_pressed;
			}
		}
	}
}

void trigger_detector_system::post_trigger_requests_from_continuous_detectors(const logic_step step) const {
	auto& cosmos = step.cosm;
	const auto delta = step.get_delta();

	cosmos.for_each(
		processing_subjects::WITH_TRIGGER_QUERY_DETECTOR,
		[&](const auto t) {
			if (!t.get<components::trigger_query_detector>().detection_intent_enabled) {
				t.get<components::processing>().disable_in(processing_subjects::WITH_TRIGGER_QUERY_DETECTOR);
			}
			else {
				messages::trigger_hit_request_message request;
				request.detector = t;
				step.transient.messages.post(request);
			}
		},
		{ subjects_iteration_flag::POSSIBLE_ITERATOR_INVALIDATION }
	);
}

void trigger_detector_system::send_trigger_confirmations(const logic_step step) const {
	auto& cosmos = step.cosm;
	const auto& physics = cosmos.systems_inferred.get<physics_system>();
	const auto& collisions = step.transient.messages.get_queue<messages::collision_message>();

	for (const auto& c : collisions) {
		if (c.type != messages::collision_message::event_type::PRE_SOLVE) {
			continue;
		}

		entity_id actual_detector = c.subject;

		const auto subject = cosmos[c.subject];

		const auto* collision_detector = subject.find<components::trigger_collision_detector>();
		const auto* const trigger = cosmos[c.collider].find<components::trigger>();
		const auto* const maybe_item = subject.find<components::item>();

		if (collision_detector == nullptr) {
			const bool is_it_arm_touching =
				maybe_item != nullptr
				&& (
					maybe_item->categories_for_slot_compatibility.test(item_category::ARM_BACK)
					|| maybe_item->categories_for_slot_compatibility.test(item_category::ARM_FRONT)
				)
			;

			if (is_it_arm_touching) {
				const auto capability = subject.get_owning_transfer_capability();

				if (capability.alive()) {
					actual_detector = capability;
					collision_detector = capability.find<components::trigger_collision_detector>();
				}
			}
		}

		if (
			collision_detector != nullptr 
			&& trigger != nullptr
			&& trigger->react_to_collision_detectors 
			&& collision_detector->detection_intent_enabled
		) {
			messages::trigger_hit_confirmation_message confirmation;
			confirmation.detector_body = actual_detector;
			confirmation.trigger = c.collider;
			step.transient.messages.post(confirmation);
		}
	}

	const auto& requests = step.transient.messages.get_queue<messages::trigger_hit_request_message>();

	for (const auto& e : requests) {
		auto& trigger_query_detector = cosmos[e.detector].get<components::trigger_query_detector>();
		const auto detector_body = cosmos[e.detector];
		
		std::unordered_set<unversioned_entity_id> found_triggers;

		ensure(detector_body.alive());

		physics.for_each_intersection_with_body(
			cosmos.get_si(),
			detector_body, 
			filters::trigger(),
			([&](const auto fixture, auto, auto) {
				const auto found_trigger_id = get_id_of_entity_of_fixture(fixture);

				const auto found_trigger = cosmos[found_trigger_id];
				const auto* const maybe_trigger = found_trigger.find<components::trigger>();

				if (maybe_trigger && maybe_trigger->react_to_query_detectors) {
					found_triggers.insert(found_trigger_id);
				}

				return callback_result::CONTINUE;
			})
		);
		
		for (const auto t : found_triggers) {
			messages::trigger_hit_confirmation_message confirmation;
			confirmation.trigger = cosmos[t];
			confirmation.detector_body = detector_body;
			trigger_query_detector.detection_intent_enabled = false;
			step.transient.messages.post(confirmation);
			break;
		}
	}
}
