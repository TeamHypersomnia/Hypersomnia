#pragma once
#include "game/detail/explosive/cascade_explosion_input.h"
#include "game/cosmos/step_declaration.h"
#include "augs/math/transform.h"
#include "game/cosmos/entity_id_declaration.h"
#include "game/detail/view_input/predictability_info.h"

namespace invariants {
	struct explosive;
}

class cosmos;

struct detonate_input {
	const logic_step& step;
	const entity_id& subject;
	const invariants::explosive& explosive;
	const transformr location;
	const predictability_info predictability = always_predictable_v;
};

void detonate(detonate_input, bool destroy_subject = true);

template <class E>
void detonate_if(const E& handle, const logic_step& step) {
	if constexpr(E::template has<invariants::explosive>()) {
		const auto& explosive = handle.template get<invariants::explosive>();

		detonate({
			step, handle.get_id(), explosive, handle.get_logic_transform()
		});
	}
}

void detonate_if(
	const entity_id&, 
	const transformr& where, 
	const logic_step& step
);
