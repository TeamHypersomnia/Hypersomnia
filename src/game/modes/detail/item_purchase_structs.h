#pragma once
#include "game/organization/special_flavour_id_types.h"
#include "game/modes/mode_commands/item_purchase.h"

using item_purchases_vector = std::vector<item_flavour_id>;


/*
	A map's limit on what the shop sells, per category.
	Disabled sells everything of the category. Enabled sells only the listed items - an empty list sells none.
	The blacklist is never sold, whatever the category.
*/

struct arena_shop_whitelist {
	// GEN INTROSPECTOR struct arena_shop_whitelist
	bool enabled = false;
	item_purchases_vector items;
	// END GEN INTROSPECTOR

	bool operator==(const arena_shop_whitelist&) const = default;
};

struct arena_spell_whitelist {
	// GEN INTROSPECTOR struct arena_spell_whitelist
	bool enabled = false;
	std::vector<spell_id> spells;
	// END GEN INTROSPECTOR

	bool operator==(const arena_spell_whitelist&) const = default;
};

struct arena_shop_whitelists {
	// GEN INTROSPECTOR struct arena_shop_whitelists
	arena_shop_whitelist firearms;
	arena_shop_whitelist melee;
	arena_shop_whitelist explosives;
	arena_shop_whitelist other_items;
	arena_spell_whitelist spells;
	item_purchases_vector blacklist;
	// END GEN INTROSPECTOR

	bool operator==(const arena_shop_whitelists&) const = default;
};
