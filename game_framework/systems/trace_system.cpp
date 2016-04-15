#include "trace_system.h"
#include "augs/entity_system/world.h"
#include "augs/entity_system/entity.h"

#include "../components/sprite_component.h"

void trace_system::lengthen_sprites_of_traces() {
	for (auto& t : targets) {
		auto& trace = t->get<components::trace>();
		auto& sprite = t->get<components::sprite>();

		if (trace.chosen_lengthening_duration_ms < 0.f) {
			trace.lengthening_time_passed_ms = 0.f;
			trace.chosen_multiplier.set(randval(trace.max_multiplier_x), randval(trace.max_multiplier_y));
			trace.chosen_lengthening_duration_ms = randval(trace.lengthening_duration_ms);
		}

		auto surplus_multiplier = trace.chosen_multiplier * trace.lengthening_time_passed_ms / trace.chosen_lengthening_duration_ms;
		sprite.size_multiplier = vec2(1, 1) + surplus_multiplier;
		sprite.center_offset = sprite.size * (surplus_multiplier / 2.f);

		trace.lengthening_time_passed_ms += delta_milliseconds();
	}
}

void trace_system::spawn_finishing_traces_for_destroyed_objects() {

}

