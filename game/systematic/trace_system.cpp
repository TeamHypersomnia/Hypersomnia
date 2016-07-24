#include "trace_system.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"

#include "game/components/trace_component.h"
#include "game/components/render_component.h"
#include "game/components/transform_component.h"
#include "game/components/sprite_component.h"
#include "game/components/damage_component.h"
#include "game/components/physics_component.h"
#include "game/components/substance_component.h"

#include "game/messages/queue_destruction.h"
#include "game/messages/will_soon_be_deleted.h"

void trace_system::lengthen_sprites_of_traces(fixed_step& step) const {
	auto& cosmos = step.cosm;
	auto delta = step.get_delta();

	for (auto t : cosmos.get(processing_subjects::WITH_TRACE)) {
		auto& trace = t.get<components::trace>();
		auto& sprite = t.get<components::sprite>();

		if (trace.chosen_lengthening_duration_ms < 0.f)
			trace.reset(cosmos.get_rng_for(t));

		vec2 surplus_multiplier;
		
		if(!trace.is_it_finishing_trace)
			surplus_multiplier = trace.chosen_multiplier * trace.lengthening_time_passed_ms / trace.chosen_lengthening_duration_ms;
		else
			surplus_multiplier = (trace.chosen_multiplier + vec2(1, 1)) * (1.f-(trace.lengthening_time_passed_ms / trace.chosen_lengthening_duration_ms)) - vec2(1, 1);

		sprite.size_multiplier = vec2(1, 1) + surplus_multiplier;

		sprite.center_offset = sprite.size * (surplus_multiplier / 2.f);

		trace.lengthening_time_passed_ms += static_cast<float>(delta.in_milliseconds());
	}
}

void trace_system::destroy_outdated_traces(fixed_step& step) const {
	auto& cosmos = step.cosm;

	for (auto t : cosmos.get(processing_subjects::WITH_TRACE)) {
		auto& trace = t.get<components::trace>();

		if (trace.lengthening_time_passed_ms > trace.chosen_lengthening_duration_ms - 0.01f) {
			trace.lengthening_time_passed_ms = trace.chosen_lengthening_duration_ms - 0.01f;

			if (trace.is_it_finishing_trace)
				step.messages.post(messages::queue_destruction(t));
		}
	}
}

void trace_system::spawn_finishing_traces_for_destroyed_objects(fixed_step& step) const {
	auto& cosmos = step.cosm;
	auto events = step.messages.get_queue<messages::will_soon_be_deleted>();

	for (auto& it : events) {
		auto e = cosmos[it.subject];

		auto* trace = e.find<components::trace>();

		if (e.has<components::substance>() && trace && !trace->is_it_finishing_trace) {
			auto finishing_trace = cosmos.create_entity("finishing_trace");
			auto copied_trace = *trace;
			copied_trace.lengthening_time_passed_ms = 0.f;
			copied_trace.chosen_lengthening_duration_ms /= 4;

			copied_trace.is_it_finishing_trace = true;
			finishing_trace += copied_trace;
			finishing_trace += e.get<components::sprite>();
			finishing_trace += e.get<components::transform>();
			finishing_trace += e.get<components::render>();

			//finishing_trace.get<components::transform>().rotation = 90;// e.get<components::physics>().velocity().degrees();

			if (e.find<components::damage>()) {
				finishing_trace.get<components::transform>().pos = e.get<components::damage>().saved_point_of_impact_before_death - 
					(e.get<components::sprite>().size/2).rotate(finishing_trace.get<components::transform>().rotation, vec2(0,0))
					;
			}

			finishing_trace.add_standard_components();
		}
	}
}

