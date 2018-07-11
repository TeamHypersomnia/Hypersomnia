#include "augs/misc/randomization.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/components/trace_component.h"
#include "game/components/render_component.h"
#include "game/components/transform_component.h"
#include "game/components/sprite_component.h"
#include "game/components/missile_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/interpolation_component.h"

#include "game/messages/interpolation_correction_request.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/will_soon_be_deleted.h"

#include "game/stateless_systems/trace_system.h"

void trace_system::lengthen_sprites_of_traces(const logic_step step) const {
	auto& cosmos = step.get_cosmos();
	const auto delta = step.get_delta();

	cosmos.for_each_having<components::trace>(
		[&](const auto t) {
			auto& trace = t.template get<components::trace>();
			const auto& trace_def = t.template get<invariants::trace>();

			vec2 surplus_multiplier;
			
			if (!trace.is_it_a_finishing_trace) {
				surplus_multiplier = trace.chosen_multiplier * trace.lengthening_time_passed_ms / trace.chosen_lengthening_duration_ms;
			}
			else {
				surplus_multiplier = (trace.chosen_multiplier + vec2(1, 1)) * (1.f - (trace.lengthening_time_passed_ms / trace.chosen_lengthening_duration_ms)) - vec2(1, 1);
			}

			const auto size_multiplier = trace_def.additional_multiplier + surplus_multiplier;

			trace.last_size_mult = size_multiplier; 
			trace.last_center_offset_mult = surplus_multiplier / 2.f;

			trace.lengthening_time_passed_ms += static_cast<float>(delta.in_milliseconds());
		}
	);
}

void trace_system::destroy_outdated_traces(const logic_step step) const {
	auto& cosmos = step.get_cosmos();

	cosmos.for_each_having<components::trace>(
		[&](const auto t) {
			auto& trace = t.template get<components::trace>();

			if (trace.lengthening_time_passed_ms > trace.chosen_lengthening_duration_ms) {
				trace.lengthening_time_passed_ms = trace.chosen_lengthening_duration_ms;

				if (trace.is_it_a_finishing_trace) {
					step.post_message(messages::queue_destruction(t));
				}
			}
		}
	);
}

void trace_system::spawn_finishing_traces_for_deleted_entities(const logic_step step) const {
	auto& cosmos = step.get_cosmos();
	const auto& events = step.get_queue<messages::will_soon_be_deleted>();

	for (const auto& it : events) {
		const auto deleted_entity = cosmos[it.subject];

		if (const auto* const trace = deleted_entity.find<components::trace>();
	   		trace && !trace->is_it_a_finishing_trace
		) {
			const auto& trace_def = deleted_entity.get<invariants::trace>();

			// TODO: Rely on create entity returning dead handle
			if (!trace_def.finishing_trace_flavour.is_set()) {
				continue;
			}
		
			auto transform_of_finishing = deleted_entity.get_logic_transform();

			if (const auto missile = deleted_entity.find<components::missile>()) {
				transform_of_finishing = missile->saved_point_of_impact_before_death;

				const auto w = deleted_entity.get_logical_size().x;
				transform_of_finishing.pos -= vec2::from_degrees(transform_of_finishing.rotation) * (w / 2);

				/* transform_of_finishing */

				/* 	- vec2(deleted_entity.get<invariants::sprite>().get_size() / 2) */
				/* 	.rotate(transform_of_finishing.rotation, vec2i(0, 0)) */
				/* ; */
			}

			if (const auto finishing_trace = cosmic::create_entity(
				cosmos, 
				trace_def.finishing_trace_flavour,
				[&](const auto typed_handle) {
					typed_handle.set_logic_transform(transform_of_finishing);
				},
				[&](const auto typed_handle) {
					{
						auto& interp = typed_handle.template get<components::interpolation>();
						interp.place_of_birth = transform_of_finishing;
					}

					{
						auto& copied_trace = typed_handle.template get<components::trace>();
						copied_trace = *trace;
						copied_trace.lengthening_time_passed_ms = 0.f;
						copied_trace.chosen_lengthening_duration_ms /= 4;
						copied_trace.is_it_a_finishing_trace = true;
					}
				}
			)) {
				messages::interpolation_correction_request request;
				request.subject = finishing_trace;
				request.set_previous_transform_from = deleted_entity;

				step.post_message(request);
			}
		}
	}
}

