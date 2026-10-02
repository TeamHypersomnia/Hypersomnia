#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "augs/drawing/sprite.h"

#include "game/cosmos/step_declaration.h"
#include "game/components/sprite_component_declaration.h"

class cosmos;
class interpolation_system;

/*
	What is left of a round's stretched sprite once it dies,
	shrinking away at the point of impact.

	Purely cosmetic, so it lives in the view instead of the cosmos -
	and shrinks smoothly every frame instead of once per step.
*/

class finishing_trace_system {
	struct finishing_trace {
		invariants::sprite sprite;

		transformr previous_transform;
		transformr impact_transform;

		vec2 initial_size_mult;
		vec2 additional_multiplier;

		uint32_t steps_passed_when_spawned = 0;
	};

	std::vector<finishing_trace> traces;

	static std::optional<vec2> calc_size_mult(
		const finishing_trace& trace,
		const double steps_since_spawned,
		const float dt_secs
	);

public:
	void acquire_new_traces(const const_logic_step step);

	/*
		Defined in finishing_trace_system.hpp.
		Calls back with the sprite, the transform to draw it at and its size multiplier.
	*/

	template <class F>
	void for_each_drawn(
		const cosmos& cosm,
		const float steps_alpha,
		F&& callback
	) const;

	void reserve_caches_for_entities(const std::size_t) const {}
	void clear();
};
