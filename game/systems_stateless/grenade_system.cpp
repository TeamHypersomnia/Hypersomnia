#include "grenade_system.h"
#include "game/transcendental/entity_id.h"
#include "augs/log.h"

#include "game/transcendental/cosmos.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/grenade_logic.h"
#include "game/detail/explosions.h"

#include "game/components/grenade_component.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/thunder_input.h"

void grenade_system::init_explosions(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto delta = step.get_delta();
	const auto now = cosmos.get_timestamp();

	cosmos.for_each(
		processing_subjects::WITH_GRENADE,
		[&](const auto it) {
			components::grenade& grenade = it.get<components::grenade>();

			if (grenade.when_explodes.was_set() && now.step >= grenade.when_explodes.step) {
				standard_explosion_input in(step);
				in.type = grenade.type;
				in.explosion_location = it.get_logic_transform();

				if (grenade.type == adverse_element_type::FORCE) {
					in.damage = 88.f;
					in.inner_ring_color = red;
					in.outer_ring_color = orange;
					in.effective_radius = 300.f;
					in.impact_force = 550.f;
					in.sound_gain = 1.8f;
					in.sound_effect = assets::sound_buffer_id::GREAT_EXPLOSION;
				}
				else if (grenade.type == adverse_element_type::PED) {
					in.damage = 88.f;
					in.inner_ring_color = cyan;
					in.outer_ring_color = turquoise;
					in.effective_radius = 350.f;
					in.impact_force = 20.f;
					in.sound_gain = 2.2f;
					in.sound_effect = assets::sound_buffer_id::PED_EXPLOSION;

					{
						for (int t = 0; t < 4; ++t) {
							static randomization rng;
							thunder_input th;

							th.delay_between_branches_ms = std::make_pair(10.f, 25.f);
							th.max_branch_lifetime_ms = std::make_pair(40.f, 65.f);
							th.branch_length = std::make_pair(10.f, 120.f);

							th.max_all_spawned_branches = 40 + (t+1)*10;
							th.max_branch_children = 2;

							th.first_branch_root = in.explosion_location;
							th.first_branch_root.pos += rng.random_point_in_circle(70.f);
							th.first_branch_root.rotation += t * 360/4;
							th.branch_angle_spread = 40.f;

							th.color = t % 2 ? cyan : turquoise;

							step.transient.messages.post(th);
						}
					}
				}
				else if (grenade.type == adverse_element_type::INTERFERENCE) {
					in.damage = 100.f;
					in.inner_ring_color = yellow;
					in.outer_ring_color = orange;
					in.effective_radius = 450.f;
					in.impact_force = 20.f;
					in.sound_gain = 2.2f;
					in.sound_effect = assets::sound_buffer_id::INTERFERENCE_EXPLOSION;
				}

				standard_explosion(
					in
				);

				step.transient.messages.post(messages::queue_destruction(it));
			}
		}
	);
}