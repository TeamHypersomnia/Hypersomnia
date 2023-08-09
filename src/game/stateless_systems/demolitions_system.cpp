#include "game/stateless_systems/demolitions_system.h"
#include "game/cosmos/entity_id.h"

#include "game/cosmos/cosmos.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"

#include "game/detail/hand_fuse_logic.h"
#include "game/detail/standard_explosion.h"

#include "game/detail/hand_fuse_math.h"

#include "game/components/explosive_component.h"
#include "game/components/hand_fuse_component.h"
#include "game/messages/queue_deletion.h"
#include "game/detail/explosive/detonate.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "augs/misc/randomization.h"

void demolitions_system::detonate_fuses(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& clk = cosm.get_clock();

	cosm.for_each_having<components::hand_fuse>(
		[&](const auto& it) {
			const auto fuse_logic = fuse_logic_provider(it, step);
			fuse_logic.advance_arming_and_defusing();

			auto& fuse = fuse_logic.fuse;
			const auto& fuse_def = fuse_logic.fuse_def;

			if (fuse.armed()) {
				{
					auto& when_beep = fuse.when_last_beep;

					if (!when_beep.was_set()) {
						when_beep = clk.now;
						/* Don't play the beep effect for the first time. */
					}
					else {
						const auto beep = beep_math { fuse, fuse_def, clk };

						if (beep.should_beep_again()) {
							fuse_def.beep_sound.start(
								step,
								sound_effect_start_input::fire_and_forget(fuse_logic.fused_transform),
								always_predictable_v
							);

							when_beep = clk.now;
						}
					}
				}

				const auto when_armed = fuse.when_armed;

				if (clk.is_ready(fuse.fuse_delay_ms, when_armed)) {
					detonate_if(it, step);
				}
			}
		}
	);
}

void demolitions_system::advance_cascade_explosions(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& clk = cosm.get_clock();

	cosm.for_each_having<components::cascade_explosion>(
		[&](const auto it) {
			const auto& cascade_def = it.template get<invariants::cascade_explosion>();
			auto& cascade = it.template get<components::cascade_explosion>();

			auto& when_next = cascade.when_next_explosion;

			if (clk.now >= when_next) {
				auto rng = cosm.get_nontemporal_rng_for(it);

				{
					const auto next_explosion_in_ms = rng.randval(cascade_def.explosion_interval_ms);
					when_next = clk.now;
					when_next.step += next_explosion_in_ms / clk.dt.in_milliseconds();
				}

				{
					const auto angle_displacement = rng.randval_h(cascade_def.max_explosion_angle_displacement);
					const auto& body = it.template get<components::rigid_body>();

					auto vel = body.get_velocity();
					vel.rotate(angle_displacement, vec2());
					body.set_velocity(vel);
				}

				auto expl_in = cascade_def.explosion;
				expl_in *= rng.randval_vm(1.f, cascade_def.explosion_scale_variation);

				expl_in.instantiate(
					step,
					it.get_logic_transform(),
					damage_cause(it)
				);

				--cascade.explosions_left;

				if (0 == cascade.explosions_left) {
					step.queue_deletion_of(it, "Cascade explosions exhausted");
				}
			}
		}
	);
}