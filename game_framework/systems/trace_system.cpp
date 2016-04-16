#include "trace_system.h"
#include "augs/entity_system/world.h"
#include "augs/entity_system/entity.h"

#include "../components/sprite_component.h"
#include "../components/damage_component.h"
#include "../messages/destroy_message.h"

void trace_system::lengthen_sprites_of_traces() {
	for (auto& t : targets) {
		auto& trace = t->get<components::trace>();
		auto& sprite = t->get<components::sprite>();

		if (trace.chosen_lengthening_duration_ms < 0.f)
			trace.reset();

		vec2 surplus_multiplier;
		
		if(!trace.is_it_finishing_trace)
			surplus_multiplier = trace.chosen_multiplier * trace.lengthening_time_passed_ms / trace.chosen_lengthening_duration_ms;
		else
			surplus_multiplier = (trace.chosen_multiplier + vec2(1, 1)) * (1.f-(trace.lengthening_time_passed_ms / trace.chosen_lengthening_duration_ms)) - vec2(1, 1);

		sprite.size_multiplier = vec2(1, 1) + surplus_multiplier;

		sprite.center_offset = sprite.size * (surplus_multiplier / 2.f);

		trace.lengthening_time_passed_ms += delta_milliseconds();

		if (trace.lengthening_time_passed_ms > trace.chosen_lengthening_duration_ms - 0.01f) {
			trace.lengthening_time_passed_ms = trace.chosen_lengthening_duration_ms - 0.01f;
			
			if (trace.is_it_finishing_trace)
				parent_world.post_message(messages::destroy_message(t));
		}
	}
}

void trace_system::spawn_finishing_traces_for_destroyed_objects() {
	auto events = parent_world.get_message_queue<messages::destroy_message>();

	for (auto& it : events) {
		auto& e = it.subject;

		auto* trace = e->find<components::trace>();

		if (trace && !trace->is_it_finishing_trace) {
			auto finishing_trace = parent_world.create_entity("finishing_trace");
			auto copied_trace = *trace;
			copied_trace.lengthening_time_passed_ms = 0.f;
			copied_trace.chosen_lengthening_duration_ms /= 4;

			copied_trace.is_it_finishing_trace = true;
			*finishing_trace += copied_trace;
			*finishing_trace += e->get<components::sprite>();
			*finishing_trace += e->get<components::transform>();
			*finishing_trace += e->get<components::render>();

			if (e->find<components::damage>()) {
				finishing_trace->get<components::transform>().pos = e->get<components::damage>().saved_point_of_impact_before_death - 
					(e->get<components::sprite>().size/2).rotate(finishing_trace->get<components::transform>().rotation, vec2(0,0))
					;
			}
		}
	}
}

