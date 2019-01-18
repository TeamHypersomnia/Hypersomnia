#include "augs/misc/randomization.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"

#include "game/components/render_component.h"
#include "game/components/transform_component.h"
#include "game/components/sprite_component.h"
#include "game/components/missile_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/interpolation_component.h"

#include "game/messages/interpolation_correction_request.h"
#include "game/messages/queue_deletion.h"
#include "game/messages/will_soon_be_deleted.h"

#include "game/stateless_systems/animation_system.h"

void animation_system::advance_stateful_animations(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto delta = step.get_delta();

	const auto& logicals = cosm.get_logical_assets();

	cosm.for_each_having<components::animation>(
		[&](const auto& t) {
			auto& animation = t.template get<components::animation>();
			const auto& animation_def = t.template get<invariants::animation>();

			if (const auto displayed_animation = logicals.find(animation_def.id)) {
				if (animation_def.loops_infinitely()) {
					animation.state.advance_looped(
						delta.in_milliseconds() * animation.speed_factor,
						displayed_animation->frames
					);
				}
				else {
					const bool finished = animation.state.advance(
						delta.in_milliseconds() * animation.speed_factor,
						displayed_animation->frames
					);

					if (finished) {
						step.queue_deletion_of(t, "Animation has finished");
					}
				}
			}
		}
	);
}
