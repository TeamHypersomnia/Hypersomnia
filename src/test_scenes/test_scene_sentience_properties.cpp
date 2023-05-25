#include "test_scenes/test_scenes_content.h"
#include "game/detail/spells/spell_structs.h"
#include "test_scenes/test_scene_sounds.h"
#include "game/cosmos/cosmos.h"
#include "test_scenes/test_scene_particle_effects.h"

#include "game/detail/spells/all_spells.h"
#include "game/detail/perks/all_perks.h"

#include "test_scenes/test_scene_images.h"
#include "test_scenes/test_scene_flavours.h"

void load_test_scene_sentience_properties(
	cosmos_common_significant& state
) {
	auto& perks = state.perks;
	auto& spells = state.spells;
	auto& meters = state.meters;

	const rgba turqoise_spell_color = turquoise;
	(void)turqoise_spell_color;
	const rgba blue_spell_border = cyan; //{ 0, 128, 209, 255 };
	const rgba green_spell_color = { 0, 255, 0, 255 };

	{
		auto& d = std::get<haste>(spells);
		d.common.cooldown_ms = 5000;
		d.common.personal_electricity_required = 36;
		d.common.associated_color = green_spell_color;
		d.common.adversarial.knockout_award = -1;
		d.perk_duration_seconds = 15;

		d.appearance.incantation = "treximo";

		d.appearance.name = "Haste";
		d.appearance.name_color = green;
		d.appearance.description = typesafe_sprintf(
			"Increases movement speed for %x seconds.", 
			d.perk_duration_seconds
		);

		d.appearance.icon = to_image_id(test_scene_image_id::SPELL_HASTE_ICON);

		d.common.cast_successful_sound.id = to_sound_id(test_scene_sound_id::CAST_SUCCESSFUL);
		d.common.cast_sparkles.id = to_particle_effect_id(test_scene_particle_effect_id::CAST_SPARKLES);
		d.common.cast_sparkles.modifier.color = d.common.associated_color;
		d.common.standard_price = static_cast<money_type>(2000);
	}

	{
		auto& d = std::get<exaltation>(spells);
		d.common.cooldown_ms = 5000;
		d.common.personal_electricity_required = 40;
		d.common.associated_color = green;
		d.common.adversarial.knockout_award = -1;

		d.appearance.incantation = "efforia";

		d.appearance.name = "Exaltation";
		d.appearance.name_color = green;
		d.appearance.description = typesafe_sprintf(
			"Restores moderate amount of health."
		);

		d.appearance.icon = to_image_id(test_scene_image_id::SPELL_EXALTATION_ICON);
		d.basic_healing_amount = 150;

		d.common.cast_successful_sound.id = to_sound_id(test_scene_sound_id::CAST_SUCCESSFUL);
		d.common.cast_sparkles.id = to_particle_effect_id(test_scene_particle_effect_id::CAST_SPARKLES);
		d.common.cast_sparkles.modifier.color = d.common.associated_color;
		d.common.cast_sparkles.modifier.scale_amounts = 1.3f;
		d.common.cast_sparkles.modifier.scale_lifetimes = 1.3f;
		d.common.standard_price = static_cast<money_type>(2800);
	}

	{
		auto& d = std::get<echoes_of_the_higher_realms>(spells);
		d.common.cooldown_ms = 2000;
		d.common.personal_electricity_required = 27;
		d.common.associated_color = yellow;
		d.common.adversarial.knockout_award = -1;

		d.appearance.incantation = "armonia";

		d.appearance.name = "Echoes of the higher realms";
		d.appearance.name_color = yellow;
		d.appearance.description = typesafe_sprintf(
			"Restores full stamina."
		);

		d.appearance.icon = to_image_id(test_scene_image_id::SPELL_ECHOES_OF_THE_HIGHER_REALMS_ICON);
		d.basic_healing_amount = 400;

		d.common.cast_successful_sound.id = to_sound_id(test_scene_sound_id::CAST_SUCCESSFUL);
		d.common.cast_sparkles.id = to_particle_effect_id(test_scene_particle_effect_id::CAST_SPARKLES);
		d.common.cast_sparkles.modifier.color = d.common.associated_color;
		d.common.cast_sparkles.modifier.scale_amounts = 1.3f;
		d.common.cast_sparkles.modifier.scale_lifetimes = 1.3f;
		d.common.standard_price = static_cast<money_type>(1100);
	}

	{
		auto& d = std::get<fury_of_the_aeons>(spells);
		d.common.cooldown_ms = 2000;
		d.common.personal_electricity_required = 30;
		d.common.associated_color = blue_spell_border;

		d.appearance.incantation = "mania aiones";

		d.appearance.name = "Fury of the Aeons";
		d.appearance.name_color = cyan;
		d.appearance.description = typesafe_sprintf(
			"Causes instant damage around the caster."
		);

		d.appearance.icon = to_image_id(test_scene_image_id::SPELL_FURY_OF_THE_AEONS_ICON);

		d.common.cast_successful_sound.id = to_sound_id(test_scene_sound_id::CAST_SUCCESSFUL);
		d.common.cast_sparkles.id = to_particle_effect_id(test_scene_particle_effect_id::CAST_SPARKLES);
		d.common.cast_sparkles.modifier.color = d.common.associated_color;
		d.common.standard_price = static_cast<money_type>(900);

		{
			auto& in = d.explosion;

			in.effective_radius = 400.f;
			in.damage.base = 88.f;
			in.damage.impact_impulse = 150.f;
			in.damage.impulse_multiplier_against_sentience = 30.f;
			in.inner_ring_color = cyan;
			in.outer_ring_color = white;
			in.sound.id = to_sound_id(test_scene_sound_id::EXPLOSION);
			in.sound.modifier.max_distance = 6000.f;
			in.sound.modifier.reference_distance = 2000.f;
			in.type = adverse_element_type::FORCE;
			in.wave_shake_radius_mult = 6.f;

			in.subject_shake.duration_ms = 1000.f;
			in.subject_shake.strength = 2.f;
			in.damage.shake = in.subject_shake;
		}
	}

	{
		auto& d = std::get<electric_triad>(spells);
		d.common.cooldown_ms = 3000;
		d.common.personal_electricity_required = 75;
		d.common.associated_color = blue_spell_border;

		d.appearance.incantation = "energeia triada";

		d.appearance.name = "Electric Triad";
		d.appearance.name_color = cyan;
		d.appearance.description = typesafe_sprintf(
			"Spawns three missiles chasing hostile entities."
		);

		d.appearance.icon = to_image_id(test_scene_image_id::SPELL_ELECTRIC_TRIAD_ICON);

		d.common.cast_successful_sound.id = to_sound_id(test_scene_sound_id::CAST_SUCCESSFUL);
		d.common.cast_sparkles.id = to_particle_effect_id(test_scene_particle_effect_id::CAST_SPARKLES);
		d.common.cast_sparkles.modifier.color = d.common.associated_color;
		d.common.standard_price = static_cast<money_type>(4100);
		d.missile_flavour = to_entity_flavour_id(test_plain_missiles::ELECTRIC_MISSILE);
	}

	{
		auto& d = std::get<ultimate_wrath_of_the_aeons>(spells);
		d.common.cooldown_ms = 2000;
		d.common.personal_electricity_required = 75;
		d.common.associated_color = blue_spell_border;

		d.appearance.incantation = "megalyteri aiones via";

		d.appearance.name = "Ultimate Wrath of the Aeons";
		d.appearance.name_color = cyan;
		d.appearance.description = typesafe_sprintf(
			"Causes massive damage around the caster after a delay."
		);

		d.appearance.icon = to_image_id(test_scene_image_id::SPELL_ULTIMATE_WRATH_OF_THE_AEONS_ICON);

		d.common.cast_successful_sound.id = to_sound_id(test_scene_sound_id::CAST_SUCCESSFUL);
		d.common.cast_sparkles.id = to_particle_effect_id(test_scene_particle_effect_id::CAST_SPARKLES);
		d.common.cast_sparkles.modifier.color = d.common.associated_color;

		d.charging_particles.id = to_particle_effect_id(test_scene_particle_effect_id::CAST_CHARGING);
		d.charging_particles.modifier.scale_lifetimes = 1.3f;
		d.charging_sound.id = to_sound_id(test_scene_sound_id::CAST_CHARGING);
		d.common.standard_price = static_cast<money_type>(2500);

		{
			standard_explosion_input in;
			in.damage.impulse_multiplier_against_sentience = 1.f;
			in.inner_ring_color = cyan;
			in.outer_ring_color = white;
			in.type = adverse_element_type::FORCE;

			{
				in.effective_radius = 400.f;
				in.damage.base = 88.f;
				in.damage.impact_impulse = 150.f;
				in.sound.modifier.max_distance = 6000.f;
				in.sound.modifier.reference_distance = 2000.f;
				in.sound.id = to_sound_id(test_scene_sound_id::EXPLOSION);

				in.subject_shake.duration_ms = 1000.f;
				in.subject_shake.strength = 1.f;
				in.damage.shake = in.subject_shake;
				in.subject_impulse = 1550.f;
				in.subject_inert_ms = 100.f;

				d.explosions[0] = in;
			}
			
			{
				in.effective_radius = 550.f;
				in.damage.base = 88.f;
				in.damage.impact_impulse = 600.f;
				in.damage.impulse_multiplier_against_sentience *= 2.5f;
				in.sound.modifier.max_distance = 6000.f;
				in.sound.modifier.reference_distance = 2000.f;
				in.sound.id = to_sound_id(test_scene_sound_id::GREAT_EXPLOSION);

				in.subject_shake.duration_ms = 1000.f;
				in.subject_shake.strength = 1.46f;
				in.damage.shake = in.subject_shake;
				in.subject_impulse = 2326.f;
				in.subject_inert_ms = 100.f;

				d.explosions[1] = in;
			}
			
			{
				in.effective_radius = 750.f;
				in.damage.base = 90.f;
				in.damage.impact_impulse = 1250.f;
				in.damage.impulse_multiplier_against_sentience *= 600.f;
				in.sound.modifier.max_distance = 6000.f;
				in.sound.modifier.reference_distance = 2000.f;
				in.sound.id = to_sound_id(test_scene_sound_id::GREAT_EXPLOSION);
				in.subject_impulse = 1938.f;
				in.subject_inert_ms = 400.f;

				in.subject_shake.duration_ms = 1000.f;
				in.subject_shake.strength = 2.f;
				in.damage.shake = in.subject_shake;

				d.explosions[2] = in;
			}
		}
	}

#if ELECTRIC_SHIELD_AS_SPELL
	{
		auto& d = std::get<electric_shield>(spells);
		d.common.cooldown_ms = 5000;
		d.perk_duration_seconds = 60;
		d.common.personal_electricity_required = 50;
		d.common.associated_color = turqoise_spell_color;

		d.appearance.incantation = "energeia aspida";

		d.appearance.name = "Electric Shield";
		d.appearance.name_color = turquoise;
		d.appearance.description = typesafe_sprintf(
			"[color=vsdarkgray]For %x seconds, damage is absorbed\nby [/color][color=cyan]Personal Electricity[/color][color=vsdarkgray] instead of [/color][color=red]Health[/color][color=vsdarkgray].[/color]",
			d.perk_duration_seconds
		);

		d.appearance.icon = to_image_id(test_scene_image_id::SPELL_ELECTRIC_SHIELD_ICON);

		d.common.cast_successful_sound.id = to_sound_id(test_scene_sound_id::CAST_SUCCESSFUL);
		d.common.cast_sparkles.id = to_particle_effect_id(test_scene_particle_effect_id::CAST_SPARKLES);
		d.common.cast_sparkles.modifier.color = d.common.associated_color;
		d.common.standard_price = static_cast<money_type>(1500);
	}
#endif

	{
		auto& p = std::get<electric_shield_perk>(perks);
		p.appearance.description = "[color=turquoise]Electric shield[/color]\n[color=vsdarkgray]Damage is absorbed by [/color][color=cyan]Personal Electricity[/color][color=vsdarkgray] instead of [/color][color=red]Health[/color][color=vsdarkgray].[/color]";
		p.appearance.icon = to_image_id(test_scene_image_id::PERK_ELECTRIC_SHIELD_ICON);
		p.appearance.bar_color = turquoise - rgba(30, 30, 30, 0);
	}

	{
		auto& p = std::get<haste_perk>(perks);
		p.appearance.description = "[color=green]Haste[/color]\n[color=vsdarkgray]You move faster.[/color]";
		p.appearance.icon = to_image_id(test_scene_image_id::PERK_HASTE_ICON);
		p.appearance.bar_color = green - rgba(30, 30, 30, 0);
	}

	{
		auto& m = std::get<health_meter>(meters);

		m.appearance.description = "[color=red]Health points:[/color] %x/%x\n[color=vsdarkgray]Stability of the physical body.[/color]";
		m.appearance.icon = to_image_id(test_scene_image_id::HEALTH_ICON);
		m.appearance.bar_color = red - rgba(30, 30, 30, 0);
	}

	{
		auto& m = std::get<personal_electricity_meter>(meters);

		m.appearance.description = "[color=cyan]Personal electricity:[/color] %x/%x\n[color=vsdarkgray]Mind-programmable matter.[/color]";
		m.appearance.icon = to_image_id(test_scene_image_id::PERSONAL_ELECTRICITY_ICON);
		m.appearance.bar_color = cyan - rgba(30, 30, 30, 0);
	}

	{
		auto& m = std::get<consciousness_meter>(meters);

		m.appearance.description = "[color=orange]Consciousness:[/color] %x/%x\n[color=vsdarkgray]Attunement of soul with the body.[/color]";
		m.appearance.icon = to_image_id(test_scene_image_id::CONSCIOUSNESS_ICON);
		m.appearance.bar_color = orange - rgba(30, 30, 30, 0);
	}
}