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

				if (grenade.type == explosion_type::FORCE) {
					in.explosion_location = it.get_logic_transform();
					in.damage = 88.f;
					in.inner_ring_color = red;
					in.outer_ring_color = orange;
					in.effective_radius = 250.f;
					in.impact_force = 550.f;
					in.sound_gain = 1.8f;
					in.sound_effect = assets::sound_buffer_id::GREAT_EXPLOSION;
				}
				else if (grenade.type == explosion_type::PED) {
					in.explosion_location = it.get_logic_transform();
					in.damage = 12.f;
					in.inner_ring_color = cyan;
					in.outer_ring_color = turquoise;
					in.effective_radius = 350.f;
					in.impact_force = 20.f;
					in.sound_gain = 2.2f;
					in.sound_effect = assets::sound_buffer_id::PED_EXPLOSION;
				}
				else if (grenade.type == explosion_type::INTERFERENCE) {
					in.explosion_location = it.get_logic_transform();
					in.damage = 12.f;
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