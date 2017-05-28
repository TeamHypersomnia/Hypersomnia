#pragma once
#include "augs/graphics/pixel.h"

#include "augs/misc/stepped_timing.h"
#include "augs/misc/enum_associative_array.h"

#include "game/transcendental/entity_id.h"

#include "game/assets/spell_id.h"

#include "game/assets/sound_buffer_id.h"
#include "game/assets/particle_effect_id.h"

#include "game/components/transform_component.h"

#include "game/detail/all_sentience_meters.h"
#include "game/detail/spells/all_spells.h"
#include "game/detail/perks/all_perks.h"

#include "game/detail/spell_logic.h"
#include "augs/misc/value_meter.h"

namespace components {
	struct sentience {
		// GEN INTROSPECTOR struct components::sentience
		augs::stepped_timestamp time_of_last_received_damage;
		augs::stepped_timestamp time_of_last_exertion;

		augs::stepped_cooldown cast_cooldown_for_all_spells;

		put_all_meter_instances_into_t<augs::trivially_copyable_tuple> meters;
		put_all_spell_instances_into_t<augs::trivially_copyable_tuple> spells;
		put_all_perk_instances_into_t<augs::trivially_copyable_tuple> perks;

		unsigned currently_casted_spell = 0xdeadbeef;
		components::transform transform_when_spell_casted;
		augs::stepped_timestamp time_of_last_spell_cast;
		augs::stepped_timestamp time_of_last_exhausted_cast;

		augs::stepped_timestamp time_of_last_shake;
		float shake_for_ms = 0.f;

		float comfort_zone = 500.f;
		float minimum_danger_amount_to_evade = 5.f;
		float danger_amount_from_hostile_attitude = 100.f;

		child_entity_id health_damage_particles;
		child_entity_id character_crosshair;

		sound_response health_decrease_sound;
		sound_response consciousness_decrease_sound;

		sound_response death_sound;
		sound_response loss_of_consciousness_sound;

		particle_effect_response health_decrease_particles;

		// END GEN INTROSPECTOR

		bool is_spell_being_cast() const {
			return currently_casted_spell != 0xdeadbeef;
		}

		rgba calculate_health_color(float time_pulse_multiplier) const;
		bool is_conscious() const;
	};
}