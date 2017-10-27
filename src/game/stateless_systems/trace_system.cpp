#include "trace_system.h"

#include "game/assets/all_logical_assets.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

#include "game/components/trace_component.h"
#include "game/components/render_component.h"
#include "game/components/transform_component.h"
#include "game/components/sprite_component.h"
#include "game/components/missile_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/all_inferred_state_component.h"
#include "game/components/interpolation_component.h"

#include "game/messages/interpolation_correction_request.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/will_soon_be_deleted.h"

void trace_system::lengthen_sprites_of_traces(const logic_step step) const {
	auto& cosmos = step.cosm;
	const auto delta = step.get_delta();
	const auto& metas = step.input.logical_assets;

	cosmos.for_each(
		processing_subjects::WITH_TRACE,
		[&](const entity_handle t) {
			auto& trace = t.get<components::trace>();
			auto& sprite = t.get<components::sprite>();

			if (trace.chosen_lengthening_duration_ms < 0.f) {
				auto rng = cosmos.get_rng_for(t);
				trace.reset(rng);
			}

			vec2 surplus_multiplier;
			
			if (!trace.is_it_a_finishing_trace) {
				surplus_multiplier = trace.chosen_multiplier * trace.lengthening_time_passed_ms / trace.chosen_lengthening_duration_ms;
			}
			else {
				surplus_multiplier = (trace.chosen_multiplier + vec2(1, 1)) * (1.f - (trace.lengthening_time_passed_ms / trace.chosen_lengthening_duration_ms)) - vec2(1, 1);
			}

			const auto original_image_size = metas.at(sprite.tex).get_size();
			const auto size_multiplier = trace.additional_multiplier + surplus_multiplier;

			sprite.size = size_multiplier * original_image_size;
			sprite.center_offset = original_image_size * (surplus_multiplier / 2.f);

			trace.lengthening_time_passed_ms += static_cast<float>(delta.in_milliseconds());
		}
	);
}

void trace_system::destroy_outdated_traces(const logic_step step) const {
	auto& cosmos = step.cosm;

	cosmos.for_each(
		processing_subjects::WITH_TRACE,
		[&](const entity_handle t) {
			auto& trace = t.get<components::trace>();

			if (trace.lengthening_time_passed_ms > trace.chosen_lengthening_duration_ms) {
				trace.lengthening_time_passed_ms = trace.chosen_lengthening_duration_ms;

				if (trace.is_it_a_finishing_trace) {
					step.transient.messages.post(messages::queue_destruction(t));
				}
			}
		}
	);
}

void trace_system::spawn_finishing_traces_for_deleted_entities(const logic_step step) const {
	auto& cosmos = step.cosm;
	const auto& events = step.transient.messages.get_queue<messages::will_soon_be_deleted>();
	const auto& metas = step.input.logical_assets;

	for (const auto& it : events) {
		const auto deleted_entity = cosmos[it.subject];

		const auto* const trace = deleted_entity.find<components::trace>();

		if (
			deleted_entity.is_inferred_state_activated()
			&& trace != nullptr
			&& !trace->is_it_a_finishing_trace
		) {
			const auto finishing_trace = cosmos.create_entity("finishing_trace");
			
			auto copied_trace = *trace;
			copied_trace.lengthening_time_passed_ms = 0.f;
			copied_trace.chosen_lengthening_duration_ms /= 4;

			const auto transform_of_deleted = deleted_entity.get_logic_transform();

			copied_trace.is_it_a_finishing_trace = true;
			finishing_trace += copied_trace;
			finishing_trace += deleted_entity.get<components::sprite>();
			finishing_trace += transform_of_deleted;
			finishing_trace += deleted_entity.get<components::render>();
			
			components::interpolation interp;
			interp.place_of_birth = transform_of_deleted;
			finishing_trace += interp;

			//finishing_trace.get<components::transform>().rotation = 90;// e.get<components::rigid_body>().velocity().degrees();

			if (deleted_entity.find<components::missile>()) {
				finishing_trace.get<components::transform>().pos = 
					deleted_entity.get<components::missile>().saved_point_of_impact_before_death
					- (deleted_entity.get<components::sprite>().get_size() / 2)
						.rotate(finishing_trace.get<components::transform>().rotation, vec2(0, 0))
				;
			}

			finishing_trace.add_standard_components(step);

			messages::interpolation_correction_request request;
			request.subject = finishing_trace;
			request.set_previous_transform_from = deleted_entity;

			step.transient.messages.post(request);
		}
	}
}

