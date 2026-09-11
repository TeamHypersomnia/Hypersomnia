#include "augs/math/repro_math.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/create_entity.hpp"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"

#include "game/components/trace_component.h"
#include "game/components/render_component.h"
#include "game/components/transform_component.h"
#include "game/components/sprite_component.h"
#include "game/components/missile_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/interpolation_component.h"

#include "game/messages/interpolation_correction_request.h"
#include "game/messages/queue_deletion.h"
#include "game/messages/will_soon_be_deleted.h"

#include "game/stateless_systems/trace_system.h"

void trace_system::lengthen_sprites_of_traces(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto delta = step.get_delta();
	const auto dt_secs = delta.in_seconds();
	const auto damp_mult = 20.f;

	cosm.for_each_having<components::trace>(
		[&](const auto t) {
			auto& trace = t.template get<components::trace>();
			const auto& trace_def = t.template get<invariants::trace>();

			if (trace.is_it_a_finishing_trace) {
				auto shrink = [&](vec2& v) {
					if (v.x == 0.f) {
						v.reset();
					}
					else {
						const auto mult = v.y / v.x;

						augs::shrink(v.x, dt_secs * damp_mult);
						augs::shrink(v.y, dt_secs * damp_mult * mult);
					}
				};

				shrink(trace.last_size_mult);

				trace.last_center_offset_mult = (trace.last_size_mult - trace_def.additional_multiplier) / 2.f;
			}
			else {
				auto surplus_multiplier = vec2(trace.chosen_multiplier * trace.lengthening_time_passed_ms / trace.chosen_lengthening_duration_ms);

				/*
					Cap the lengthwise stretch so that the sprite's rear tip never reaches
					behind the point of the shot. The rear extends w * (1 + s)^2 / 2 behind
					the body (with additional_multiplier = 1), while the body has only travelled
					speed * time - and the rendered, interpolated position lags up to one tick
					behind the logical one, so one tick's worth of distance is subtracted.
					Otherwise, with short lengthening durations and low tickrates, the stretched
					trace - and the trail streams anchored at its back - poked into the shooter.
				*/
				{
					const auto w = static_cast<float>(t.get_logical_size().x);
					const auto speed = t.get_effective_velocity().length();

					const auto travelled = speed * std::max(0.f, trace.lengthening_time_passed_ms - static_cast<float>(delta.in_milliseconds())) / 1000.f;

					if (w > 0.f) {
						const auto max_surplus = repro::sqrt(std::max(0.f, 2.f * travelled / w)) - 1.f;
						surplus_multiplier.x = std::min(surplus_multiplier.x, std::max(0.f, max_surplus));
					}
				}

				const auto size_multiplier = trace_def.additional_multiplier + surplus_multiplier;

				trace.last_size_mult = size_multiplier;
				trace.last_center_offset_mult = surplus_multiplier / 2.f;

				trace.lengthening_time_passed_ms += static_cast<float>(delta.in_milliseconds());
			}
		}
	);
}

void trace_system::destroy_outdated_traces(const logic_step step) const {
	auto& cosm = step.get_cosmos();

	cosm.for_each_having<components::trace>(
		[&](const auto t) {
			auto& trace = t.template get<components::trace>();

			if (trace.lengthening_time_passed_ms > trace.chosen_lengthening_duration_ms) {
				trace.lengthening_time_passed_ms = trace.chosen_lengthening_duration_ms;
			}

			if (trace.is_it_a_finishing_trace) {
				if (trace.last_size_mult.length_sq() < 0.00001f) {
					step.queue_deletion_of(t, "Trace expiration");
				}
			}
		}
	);
}

void trace_system::spawn_finishing_traces_for_deleted_entities(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto& events = step.get_queue<messages::will_soon_be_deleted>();

	auto access = allocate_new_entity_access();

	for (const auto& it : events) {
		const auto deleted_entity = cosm[it.subject];

		deleted_entity.dispatch_on_having_all<invariants::trace>([&](const auto& typed_deleted) {
			const auto trace = typed_deleted.template get<components::trace>();

			if (trace.is_it_a_finishing_trace) {
				return;
			}

			const auto& trace_def = typed_deleted.template get<invariants::trace>();

			bool during_penetration = false;
			auto transform_of_finishing = typed_deleted.get_logic_transform();

			if (const auto missile = typed_deleted.template find<components::missile>()) {
				transform_of_finishing = missile->saved_point_of_impact_before_death;
				during_penetration = missile->during_penetration;

				const auto w = typed_deleted.get_logical_size().x;
				transform_of_finishing.pos -= transform_of_finishing.get_direction() * (w / 2);

				/* transform_of_finishing */

				/* 	- vec2(typed_deleted.get<invariants::sprite>().get_size() / 2) */
				/* 	.rotate(transform_of_finishing.rotation, vec2i(0, 0)) */
				/* ; */
			}

			auto src_interp = get_corresponding<components::interpolation>(typed_deleted);

			if (const auto finishing_trace = cosmic::create_entity(
				access,
				cosm, 
				trace_def.finishing_trace_flavour,
				[transform_of_finishing](const auto typed_handle, auto&&...) {
					typed_handle.set_logic_transform(transform_of_finishing);
				},
				[transform_of_finishing, trace, during_penetration](const auto typed_handle) {
					{
						auto& interp = get_corresponding<components::interpolation>(typed_handle);
						interp.snap_to(transform_of_finishing);
					}

					{
						auto& copied_trace = typed_handle.template get<components::trace>();
						copied_trace = trace;
						copied_trace.is_it_a_finishing_trace = true;
						copied_trace.during_penetration = during_penetration;
					}
				}
			)) {
				finishing_trace.template dispatch_on_having_all<invariants::interpolation>(
					[src_interp](const auto& typed_finishing) {
						auto& trg_interp = get_corresponding<components::interpolation>(typed_finishing);
						trg_interp = src_interp;
					}
				);
			}
		});
	}
}

