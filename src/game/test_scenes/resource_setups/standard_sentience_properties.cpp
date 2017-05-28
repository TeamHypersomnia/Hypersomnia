#include "generated/setting_build_test_scenes.h"
#if BUILD_TEST_SCENES
#include "all.h"
#include "game/detail/spells/spell_structs.h"
#include "game/transcendental/cosmos.h"

#include "game/detail/spells/all_spells.h"
#include "game/detail/perks/all_perks.h"

void set_standard_sentience_props(
	cosmos_global_state& state
) {
	auto& perks = state.perks;
	auto& spells = state.spells;
	auto& meters = state.meters;

	const rgba turqoise_spell_color = turquoise;
	const rgba blue_spell_border = cyan; //{ 0, 128, 209, 255 };
	const rgba green_spell_color = { 0, 200, 0, 255 };

	{
		auto& d = std::get<haste>(spells);
		d.common.cooldown_ms = 5000;
		d.common.personal_electricity_required = 60;
		d.common.associated_color = green_spell_color;
		d.perk_duration_seconds = 33;

		d.appearance.incantation = L"treximo";

		d.appearance.name = L"[color=green]Haste[/color]";
		d.appearance.description = typesafe_sprintf(
			L"[color=vsdarkgray]Increases movement speed for %x seconds.[/color]", 
			d.perk_duration_seconds
		);

		d.appearance.icon = assets::game_image_id::SPELL_HASTE_ICON;
	}

	{
		auto& d = std::get<fury_of_the_aeons>(spells);
		d.common.cooldown_ms = 2000;
		d.common.personal_electricity_required = 100;
		d.common.associated_color = blue_spell_border;

		d.appearance.incantation = L"mania aiones";

		d.appearance.name = L"[color=cyan]Fury of the Aeons[/color]";
		d.appearance.description = typesafe_sprintf(
			L"[color=vsdarkgray]Causes instant damage around the caster.[/color]"
		);

		d.appearance.icon = assets::game_image_id::SPELL_FURY_OF_THE_AEONS_ICON;
	}

	{
		auto& d = std::get<electric_triad>(spells);
		d.common.cooldown_ms = 3000;
		d.common.personal_electricity_required = 120;
		d.common.associated_color = blue_spell_border;

		d.appearance.incantation = L"energeia triada";

		d.appearance.name = L"[color=cyan]Electric Triad[/color]";
		d.appearance.description = typesafe_sprintf(
			L"[color=vsdarkgray]Spawns three electric missiles\nhoming towards hostile entities.[/color]"
		);

		d.appearance.icon = assets::game_image_id::SPELL_ELECTRIC_TRIAD_ICON;
	}

	{
		auto& d = std::get<ultimate_wrath_of_the_aeons>(spells);
		d.common.cooldown_ms = 2000;
		d.common.casting_time_ms = 3000;
		d.common.personal_electricity_required = 260;
		d.common.associated_color = blue_spell_border;

		d.appearance.incantation = L"megalyteri aiones via";

		d.appearance.name = L"[color=cyan]Ultimate Wrath of the Aeons[/color]";
		d.appearance.description = typesafe_sprintf(
			L"[color=vsdarkgray]Causes massive damage around the caster.\nRequires delay to initiate.[/color]"
		);

		d.appearance.icon = assets::game_image_id::SPELL_ULTIMATE_WRATH_OF_THE_AEONS_ICON;
	}

	{
		auto& d = std::get<electric_shield>(spells);
		d.common.cooldown_ms = 5000;
		d.perk_duration_seconds = 60;
		d.common.personal_electricity_required = 50;
		d.common.associated_color = turqoise_spell_color;

		d.appearance.incantation = L"energeia aspida";

		d.appearance.name = L"[color=turquoise]Electric Shield[/color]";
		d.appearance.description = typesafe_sprintf(
			L"[color=vsdarkgray]For %x seconds, damage is absorbed\nby [/color][color=cyan]Personal Electricity[/color][color=vsdarkgray] instead of [/color][color=red]Health[/color][color=vsdarkgray].[/color]",
			d.perk_duration_seconds
		);

		d.appearance.icon = assets::game_image_id::SPELL_ELECTRIC_SHIELD_ICON;
	}

	{
		auto& p = std::get<electric_shield_perk>(perks);
		p.appearance.description = L"[color=turquoise]Electric shield[/color]\n[color=vsdarkgray]Damage is absorbed by [/color][color=cyan]Personal Electricity[/color][color=vsdarkgray] instead of [/color][color=red]Health[/color][color=vsdarkgray].[/color]";
		p.appearance.icon = assets::game_image_id::PERK_ELECTRIC_SHIELD_ICON;
		p.appearance.bar_color = turquoise - rgba(30, 30, 30, 0);
	}

	{
		auto& p = std::get<haste_perk>(perks);
		p.appearance.description = L"[color=green]Haste[/color]\n[color=vsdarkgray]You move faster.[/color]";
		p.appearance.icon = assets::game_image_id::PERK_HASTE_ICON;
		p.appearance.bar_color = green - rgba(30, 30, 30, 0);
	}

	{
		auto& m = std::get<health_meter>(meters);

		m.appearance.description = L"[color=red]Healsth points:[/color] %x/%x\n[color=vsdarkgray]Stability of the physical body.[/color]";
		m.appearance.icon = assets::game_image_id::HEALTH_ICON;
		m.appearance.bar_color = red - rgba(30, 30, 30, 0);
	}

	{
		auto& m = std::get<personal_electricity_meter>(meters);

		m.appearance.description = L"[color=cyan]Personal electricity:[/color] %x/%x\n[color=vsdarkgray]Mind-programmable matter.[/color]";
		m.appearance.icon = assets::game_image_id::PERSONAL_ELECTRICITY_ICON;
		m.appearance.bar_color = cyan - rgba(30, 30, 30, 0);
	}

	{
		auto& m = std::get<consciousness_meter>(meters);

		m.appearance.description = L"[color=orange]Consciousness:[/color] %x/%x\n[color=vsdarkgray]Attunement of soul with the body.[/color]";
		m.appearance.icon = assets::game_image_id::CONSCIOUSNESS_ICON;
		m.appearance.bar_color = orange - rgba(30, 30, 30, 0);
	}
}
#endif