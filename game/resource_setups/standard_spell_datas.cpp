#include "all.h"

#include "game/transcendental/cosmos.h"

void set_standard_spell_properties(cosmos& spells) {
	{
		auto& d = spells[spell_type::HASTE];
		d.cooldown_ms = 5000;
		d.incantation = L"treximo";
		d.perk_duration_seconds = 33;
		d.personal_electricity_required = 60;
	}

	{
		auto& d = spells[spell_type::FURY_OF_THE_AEONS];
		d.cooldown_ms = 2000;
		d.incantation = L"mania aiones";
		d.personal_electricity_required = 100;
	}

	{
		auto& d = spells[spell_type::ELECTRIC_TRIAD];
		d.cooldown_ms = 3000;
		d.incantation = L"energeia triada";
		d.personal_electricity_required = 120;
	}

	{
		auto& d = spells[spell_type::ULTIMATE_WRATH_OF_THE_AEONS];
		d.cooldown_ms = 2000;
		d.casting_time_ms = 3000;
		d.incantation = L"megalyteri aiones via";
		d.personal_electricity_required = 260;
	}

	{
		auto& d = spells[spell_type::ELECTRIC_SHIELD];
		d.cooldown_ms = 5000;
		d.incantation = L"energeia aspida";
		d.perk_duration_seconds = 60;
		d.personal_electricity_required = 50;
	}
}
