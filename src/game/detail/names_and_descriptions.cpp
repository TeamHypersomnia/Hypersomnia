#include "game/enums/slot_function.h"
#include "game/enums/item_category.h"
#include "game/common_state/entity_name_str.h"

entity_name_str get_bbcoded_item_categories(const item_category_flagset& flags) {
	entity_name_str result;

	if (flags.test(item_category::MAGAZINE)) {
		result += "Magazine, ";
	}
	if (flags.test(item_category::MUZZLE_ATTACHMENT)) {
		result += "Muzzle attachment, ";
	}
	if (flags.test(item_category::RAIL_ATTACHMENT)) {
		result += "Rail attachment, ";
	}
	if (flags.test(item_category::SHOT_CHARGE)) {
		result += "Shot charge, ";
	}
	if (flags.test(item_category::BACK_WEARABLE)) {
		result += "Back container, ";
	}
	if (flags.test(item_category::BELT_WEARABLE)) {
		result += "Belt wearable, ";
	}
	if (flags.test(item_category::TORSO_ARMOR)) {
		result += "Torso armor, ";
	}

	if (!result.empty()) {
		result = result.substr(0, result.length() - 2);
	}
	else {
		result = "Everything";
	}

	return result;
}

entity_name_str get_bbcoded_slot_function_name(const slot_function f) {
	switch (f) {
	case slot_function::GUN_CHAMBER:
		return{
			"Chamber",
		};
	case slot_function::GUN_CHAMBER_MAGAZINE:
		return{
			"Integral magazine",
		};
	case slot_function::GUN_DETACHABLE_MAGAZINE:
		return{
			"Magazine slot",
		};
	case slot_function::GUN_RAIL:
		return{
			"Rail",
		};
	case slot_function::GUN_MUZZLE:
		return{
			"Muzzle",
		};
	case slot_function::PRIMARY_HAND:
		return{
			"Primary arm",
		};
	case slot_function::SECONDARY_HAND:
		return{
			"Secondary arm",
		};
	case slot_function::TORSO_ARMOR:
		return{
			"Torso",
		};
	case slot_function::BACK:
		return{
			"Back",
		};
	case slot_function::OVER_BACK:
		return{
			"Over back",
		};
	case slot_function::SHOULDER:
		return{
			"Shoulder",
		};
	case slot_function::BELT:
		return{
			"Belt",
		};
	case slot_function::PERSONAL_DEPOSIT:
		return{
			"Personal deposit",
		};
	default: return{ "Unknown" };
	}
}

entity_name_str get_bbcoded_slot_function_description(const slot_function f) {
	switch (f) {
	case slot_function::GUN_CHAMBER:
		return{
			"Single charge is inserted here\nprior to being fired."
		};
	case slot_function::GUN_CHAMBER_MAGAZINE:
		return{
			"Internal storage for charges\nremoving the need for detachable mag."
		};
	case slot_function::GUN_DETACHABLE_MAGAZINE:
		return{
			"Detachable magazine goes here."
		};
	case slot_function::GUN_RAIL:
		return{
			" "
		};
	case slot_function::GUN_MUZZLE:
		return{
			"Various gunshot modifiers go here."
		};
	case slot_function::PRIMARY_HAND:
		return{
			"Primary hand."
		};
	case slot_function::SECONDARY_HAND:
		return{
			"Secondary hand."
		};
	case slot_function::TORSO_ARMOR:
		return{
			"Clothing or armors go here."
		};
	case slot_function::BACK:
		return{
			"For backpacks."
		};
	case slot_function::OVER_BACK:
		return{
			"For bombs and other heavy objects."
		};
	case slot_function::SHOULDER:
		return{
			"For knives."
		};
	case slot_function::BELT:
		return{
			"For defuser kits and other belt-wearables."
		};
	case slot_function::PERSONAL_DEPOSIT:
		return{
			"For several items at most.",
		};
	default: return{ "Unknown" };
	}
}
