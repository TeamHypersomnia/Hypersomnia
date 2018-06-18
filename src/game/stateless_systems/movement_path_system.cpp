#include "augs/misc/randomization.h"

#include "game/debug_utils.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/components/render_component.h"
#include "game/components/transform_component.h"
#include "game/components/sprite_component.h"
#include "game/components/missile_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/interpolation_component.h"

#include "game/messages/interpolation_correction_request.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/will_soon_be_deleted.h"

#include "game/stateless_systems/movement_path_system.h"

void movement_path_system::advance_paths(const logic_step step) const {
	auto& cosmos = step.get_cosmos();
	const auto delta = step.get_delta();

	auto& npo = cosmos.get_solvable_inferred({}).tree_of_npo;

	cosmos.for_each_having<components::movement_path>(
		[&](const auto t) {
			//auto& movement_path = t.template get<components::movement_path>();
			const auto& movement_path_def = t.template get<invariants::movement_path>();

			auto& transform = t.template get<components::transform>();
			transform.rotation += movement_path_def.continuous_rotation_speed * delta.in_seconds();

			npo.infer_cache_for(t);
		}
	);
}
