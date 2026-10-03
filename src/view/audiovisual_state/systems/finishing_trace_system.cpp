#include <algorithm>
#include "augs/templates/container_templates.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/get_corresponding.h"
#include "game/components/missile_component.h"
#include "game/components/interpolation_component.h"
#include "game/detail/calc_trace_scaling.h"
#include "game/messages/will_soon_be_deleted.h"

#include "view/audiovisual_state/systems/finishing_trace_system.h"

/*
	How many of the trace's widths it loses per second.
*/

constexpr float finishing_trace_shrink_speed = 20.f;

void finishing_trace_system::clear() {
	traces.clear();
}

std::optional<vec2> finishing_trace_system::calc_size_mult(
	const finishing_trace& trace,
	const double steps_since_spawned,
	const float dt_secs
) {
	const auto& initial = trace.initial_size_mult;

	if (initial.x == 0.f) {
		return std::nullopt;
	}

	/*
		Shrinks lengthwise linearly over time,
		the other dimension keeping its proportion to the length.
	*/

	const auto shrunk_by = static_cast<float>(std::max(0.0, steps_since_spawned)) * dt_secs * finishing_trace_shrink_speed;
	const auto shrunk_x = std::max(0.f, initial.x - shrunk_by);
	const auto result = initial * (shrunk_x / initial.x);

	if (result.length_sq() < 0.00001f) {
		return std::nullopt;
	}

	return result;
}

void finishing_trace_system::acquire_new_traces(const const_logic_step step) {
	const auto& cosm = step.get_cosmos();
	const auto steps_passed = cosm.get_total_steps_passed();
	const auto dt_secs = cosm.get_clock().get_bullet_dt().in_seconds();

	/*
		Even the start of the interpolated shrinking is gone by now.
		Also drops the traces of a cosmos that was rewound or replaced.
	*/

	erase_if(traces, [&](const finishing_trace& t) {
		if (steps_passed < t.steps_passed_when_spawned) {
			return true;
		}

		const auto steps_since_spawned = static_cast<double>(steps_passed - t.steps_passed_when_spawned);
		return calc_size_mult(t, steps_since_spawned - 1.0, dt_secs) == std::nullopt;
	});

	/*
		Deletions are performed only after the post-solve,
		so the dying rounds can still be read here.
	*/

	const auto& deletions = step.get_queue<messages::will_soon_be_deleted>();

	for (const auto& e : deletions) {
		const auto subject = cosm[e.subject];

		if (subject.dead()) {
			continue;
		}

		subject.dispatch_on_having_all<invariants::trace>([&](const auto& typed_subject) {
			const auto& trace_def = typed_subject.template get<invariants::trace>();

			if (!trace_def.finishing_sprite.image_id.is_set()) {
				return;
			}

			/*
				The stretch at the end of the step the round died in.
			*/

			const auto scaling = ::calc_trace_scaling(typed_subject, 1.0f);

			if (scaling == std::nullopt) {
				return;
			}

			auto impact_transform = typed_subject.get_logic_transform();

			if (const auto missile = typed_subject.template find<components::missile>()) {
				impact_transform = missile->saved_point_of_impact_before_death;

				const auto w = typed_subject.get_logical_size().x;
				impact_transform.pos -= impact_transform.get_direction() * (w / 2);
			}

			/*
				Glides from where the round was last drawn heading to,
				exactly like the round's own interpolation would.
			*/

			const auto& interp = get_corresponding<components::interpolation>(typed_subject);

			finishing_trace new_trace;
			new_trace.sprite = trace_def.finishing_sprite;
			new_trace.previous_transform = interp.desired_transform;
			new_trace.impact_transform = impact_transform;
			new_trace.initial_size_mult = scaling->size_mult;
			new_trace.additional_multiplier = trace_def.additional_multiplier;
			new_trace.steps_passed_when_spawned = steps_passed;

			traces.emplace_back(std::move(new_trace));
		});
	}
}
