#include "generated/setting_build_test_scenes.h"
#if BUILD_TEST_SCENES
#include "all.h"
#include "game/assets/spell_id.h"
#include "game/assets/spell.h"
#include "game/transcendental/cosmos.h"

void set_standard_spell_properties(assets_manager& spells) {
	const rgba turqoise_spell_color = turquoise;
	const rgba blue_spell_border = cyan; //{ 0, 128, 209, 255 };
	const rgba green_spell_color = { 0, 200, 0, 255 };

	{
		auto& d = spells[assets::spell_id::HASTE];
		d.logical.cooldown_ms = 5000;
		d.logical.perk_duration_seconds = 33;
		d.logical.personal_electricity_required = 60;
		d.logical.border_col = green_spell_color;

		d.incantation = L"treximo";

		d.spell_name = L"[color=green]Haste[/color]";
		d.spell_description = typesafe_sprintf(
			L"[color=vsdarkgray]Increases movement speed for %x seconds.[/color]", 
			d.logical.perk_duration_seconds
		);

		d.icon = assets::game_image_id::SPELL_HASTE_ICON;
	}

	{
		auto& d = spells[assets::spell_id::FURY_OF_THE_AEONS];
		d.logical.cooldown_ms = 2000;
		d.logical.personal_electricity_required = 100;
		d.logical.border_col = blue_spell_border;

		d.incantation = L"mania aiones";

		d.spell_name = L"[color=cyan]Fury of the Aeons[/color]";
		d.spell_description = typesafe_sprintf(
			L"[color=vsdarkgray]Causes instant damage around the caster.[/color]"
		);

		d.icon = assets::game_image_id::SPELL_FURY_OF_THE_AEONS_ICON;
	}

	{
		auto& d = spells[assets::spell_id::ELECTRIC_TRIAD];
		d.logical.cooldown_ms = 3000;
		d.logical.personal_electricity_required = 120;
		d.logical.border_col = blue_spell_border;

		d.incantation = L"energeia triada";

		d.spell_name = L"[color=cyan]Electric Triad[/color]";
		d.spell_description = typesafe_sprintf(
			L"[color=vsdarkgray]Spawns three electric missiles\nhoming towards hostile entities.[/color]"
		);

		d.icon = assets::game_image_id::SPELL_ELECTRIC_TRIAD_ICON;
	}

	{
		auto& d = spells[assets::spell_id::ULTIMATE_WRATH_OF_THE_AEONS];
		d.logical.cooldown_ms = 2000;
		d.logical.casting_time_ms = 3000;
		d.logical.personal_electricity_required = 260;
		d.logical.border_col = blue_spell_border;

		d.incantation = L"megalyteri aiones via";

		d.spell_name = L"[color=cyan]Ultimate Wrath of the Aeons[/color]";
		d.spell_description = typesafe_sprintf(
			L"[color=vsdarkgray]Causes massive damage around the caster.\nRequires delay to initiate.[/color]"
		);

		d.icon = assets::game_image_id::SPELL_ULTIMATE_WRATH_OF_THE_AEONS_ICON;
	}

	{
		auto& d = spells[assets::spell_id::ELECTRIC_SHIELD];
		d.logical.cooldown_ms = 5000;
		d.logical.perk_duration_seconds = 60;
		d.logical.personal_electricity_required = 50;
		d.logical.border_col = turqoise_spell_color;

		d.incantation = L"energeia aspida";

		d.spell_name = L"[color=turquoise]Electric Shield[/color]";
		d.spell_description = typesafe_sprintf(
			L"[color=vsdarkgray]For %x seconds, damage is absorbed\nby [/color][color=cyan]Personal Electricity[/color][color=vsdarkgray] instead of [/color][color=red]Health[/color][color=vsdarkgray].[/color]",
			d.logical.perk_duration_seconds
		);

		d.icon = assets::game_image_id::SPELL_ELECTRIC_SHIELD_ICON;
	}
}
#endif