#include "spell_data.h"

spell_data get_spell_data(const spell_type spell) {
	switch (spell) {
	case spell_type::HASTE: return { 60, 5000, L"treximo" };
	case spell_type::FURY_OF_THE_AEONS: return { 100, 2000, L"mania aiones" };
	case spell_type::ELECTRIC_TRIAD: return { 120, 3000, L"energeia triada" };
	case spell_type::ULTIMATE_WRATH_OF_THE_AEONS: return { 260, 2000, L"megalyteri aiones via" };
	case spell_type::ELECTRIC_SHIELD: return { 50, 5000, L"energeia aspida" };
	default: LOG("Unknown spell: %x", static_cast<int>(spell)); return {};
	}
}