#include "spell_logic.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/components/sentience_component.h"
#include "game/components/render_component.h"
#include "game/components/wandering_pixels_component.h"
#include "game/components/missile_component.h"
#include "game/components/sender_component.h"
#include "game/messages/create_particle_effect.h"
#include "game/systems_stateless/particles_existence_system.h"
#include "game/systems_stateless/sound_existence_system.h"

#include "game/ingredients/ingredients.h"
#include "game/assets/sound_buffer_id.h"

#include "game/detail/entity_scripts.h"
#include "game/enums/filters.h"

#include "game/detail/explosions.h"
#include "game/detail/spells/spell_structs.h"

bool are_additional_conditions_for_casting_fulfilled(
	const assets::spell_id spell,
	const const_entity_handle caster
) {
	switch (spell) {
	case assets::spell_id::ELECTRIC_TRIAD: {

	}

	default:
		return true;
	}
}

void perform_spell_logic(
	const logic_step step,
	const assets::spell_id spell,
	const entity_handle caster,
	components::sentience& sentience,
	const augs::stepped_timestamp when_casted,
	const augs::stepped_timestamp now
) {
	auto& cosmos = step.cosm;
	const auto& metas = step.input.metas_of_assets;

	const auto spell_data = metas[spell];
	const auto dt = cosmos.get_fixed_delta();
	const auto caster_transform = caster.get_logic_transform();
	
	const auto ignite_sparkle_particles = [&]() {
		particles_existence_input burst;

		burst.effect.id = assets::particle_effect_id::CAST_SPARKLES;
		burst.effect.modifier.colorize = spell_data.border_col;
		
		burst.create_particle_effect_entity(
			step,
			caster_transform,
			caster
		).add_standard_components(step);
	};

	const auto ignite_charging_particles = [&](const rgba col) {
		particles_existence_input burst;

		burst.effect.id = assets::particle_effect_id::CAST_CHARGING;
		burst.effect.modifier.colorize = col;
		burst.effect.modifier.scale_lifetimes = 1.3f;
		burst.effect.modifier.homing_target = caster;

		burst.create_particle_effect_entity(
			step,
			caster_transform,
			caster
		).add_standard_components(step);
	};
	
	const auto play_sound = [&](const assets::sound_buffer_id effect, const float gain = 1.f) {
		sound_existence_input in;
		in.delete_entity_after_effect_lifetime = true;
		in.direct_listener = caster;
		in.effect.id = effect;
		in.effect.modifier.gain = gain;

		in.create_sound_effect_entity(step, caster_transform, entity_id()).add_standard_components(step);
	};

	const auto play_standard_sparkles_sound = [&]() {
		play_sound(assets::sound_buffer_id::CAST_SUCCESSFUL);
	};

	switch (spell) {
	case assets::spell_id::HASTE:
		ignite_sparkle_particles();
		play_standard_sparkles_sound();

		sentience.haste.perk.set_for_duration(static_cast<float>(spell_data.perk_duration_seconds * 1000), now); 

		break;

	case assets::spell_id::ELECTRIC_SHIELD:
		ignite_sparkle_particles();
		play_standard_sparkles_sound();

		sentience.electric_shield.perk.set_for_duration(static_cast<float>(spell_data.perk_duration_seconds * 1000), now); 

		break;

	case assets::spell_id::FURY_OF_THE_AEONS:
		if (when_casted == now) {
			ignite_sparkle_particles();
			play_standard_sparkles_sound();

			sentience.shake_for_ms = 400.f;
			sentience.time_of_last_shake = now;

			standard_explosion_input in;
			in.effective_radius = 250.f;
			in.damage = 88.f;
			in.impact_force = 150.f;
			in.inner_ring_color = cyan;
			in.outer_ring_color = white;
			in.sound_effect = assets::sound_buffer_id::EXPLOSION;
			in.sound_gain = 1.2f;
			in.type = adverse_element_type::FORCE;

			in.instantiate(step, caster_transform, caster);
		}

		break;

	case assets::spell_id::ULTIMATE_WRATH_OF_THE_AEONS:
	{
		const auto seconds_passed = (now - when_casted).in_seconds(dt);
		const auto first_at = augs::stepped_timestamp{ when_casted.step + static_cast<unsigned>(1.3f / dt.in_seconds()) };
		const auto second_at = augs::stepped_timestamp{ when_casted.step + static_cast<unsigned>(1.8f / dt.in_seconds()) };
		const auto third_at = augs::stepped_timestamp{ when_casted.step + static_cast<unsigned>(2.3f / dt.in_seconds()) };
		
		standard_explosion_input in;
		in.damage = 88.f;
		in.inner_ring_color = cyan;
		in.outer_ring_color = white;
		in.type = adverse_element_type::FORCE;

		if (now == when_casted) {
			ignite_sparkle_particles();
			ignite_charging_particles(cyan);
			ignite_charging_particles(white);
			play_standard_sparkles_sound();
			play_sound(assets::sound_buffer_id::CAST_CHARGING);
		}
		else if (now == first_at) {
			sentience.shake_for_ms = 400.f;
			sentience.time_of_last_shake = now;
			
			in.effective_radius = 200.f;
			in.impact_force = 150.f;
			in.sound_gain = 1.2f;
			in.sound_effect = assets::sound_buffer_id::EXPLOSION;

			in.instantiate(step, caster_transform, caster);
		}
		else if (now == second_at) {
			sentience.shake_for_ms = 500.f;
			sentience.time_of_last_shake = now;

			in.effective_radius = 400.f;
			in.impact_force = 200.f;
			in.sound_gain = 1.0f;
			in.sound_effect = assets::sound_buffer_id::GREAT_EXPLOSION;

			in.instantiate(step, caster_transform, caster);
		}
		else if (now == third_at) {
			sentience.shake_for_ms = 600.f;
			sentience.time_of_last_shake = now;

			in.effective_radius = 600.f;
			in.impact_force = 250.f;
			in.sound_gain = 1.2f;
			in.sound_effect = assets::sound_buffer_id::GREAT_EXPLOSION;

			in.instantiate(step, caster_transform, caster);
		}

		break;
	}
	case assets::spell_id::ELECTRIC_TRIAD:
		ignite_sparkle_particles();
		play_standard_sparkles_sound();

		}

		break;

	default: 
		LOG("Unknown spell: %x", static_cast<int>(spell)); 
		break;
	}
}

