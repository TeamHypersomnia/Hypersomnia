#include "entity_description.h"
#include "game/enums/slot_function.h"
#include "game/enums/item_category.h"
#include "augs/ensure.h"

std::wstring get_bbcoded_item_categories(const item_category_flagset& flags) {
	std::wstring result;

	if (flags.test(item_category::MAGAZINE)) {
		result += L"Magazine, ";
	}
	if (flags.test(item_category::MUZZLE_ATTACHMENT)) {
		result += L"Muzzle attachment, ";
	}
	if (flags.test(item_category::RAIL_ATTACHMENT)) {
		result += L"Rail attachment, ";
	}
	if (flags.test(item_category::SHOT_CHARGE)) {
		result += L"Shot charge, ";
	}
	if (flags.test(item_category::SHOULDER_CONTAINER)) {
		result += L"Shoulder container, ";
	}
	if (flags.test(item_category::TORSO_ARMOR)) {
		result += L"Torso armor, ";
	}
	if (flags.test(item_category::ARM_BACK)) {
		result += L"Arm's back, ";
	}
	if (flags.test(item_category::ARM_FRONT)) {
		result += L"Arm's front, ";
	}

	if (!result.empty()) {
		result = result.substr(0, result.length() - 2);
	}
	else {
		result = L"Everything";
	}

	return result;
}

std::wstring get_bbcoded_slot_function_name(const slot_function f) {
	switch (f) {
	case slot_function::GUN_CHAMBER:
		return{
			L"Chamber",
		};
	case slot_function::GUN_CHAMBER_MAGAZINE:
		return{
			L"Integral magazine",
		};
	case slot_function::GUN_DETACHABLE_MAGAZINE:
		return{
			L"Magazine slot",
		};
	case slot_function::GUN_RAIL:
		return{
			L"Rail",
		};
	case slot_function::GUN_MUZZLE:
		return{
			L"Muzzle",
		};
	case slot_function::WIELDED_ITEM:
		return{
			L"Hand",
		};
	case slot_function::PRIMARY_ARM_BACK:
		return{
			L"Primary arm",
		};
	case slot_function::SECONDARY_ARM_BACK:
		return{
			L"Secondary arm",
		};
	case slot_function::TORSO_ARMOR:
		return{
			L"Torso",
		};
	case slot_function::SHOULDER:
		return{
			L"Shoulder",
		};
	case slot_function::ARM_FRONT:
		return{
			L"Arm's front",
		};
	default: return{ L"Unknown" };
	}
}

std::wstring get_bbcoded_slot_function_details(const slot_function f) {
	switch (f) {
	case slot_function::GUN_CHAMBER:
		return{
			L"Single charge is inserted here\nprior to being fired."
		};
	case slot_function::GUN_CHAMBER_MAGAZINE:
		return{
			L"Internal storage for charges\nremoving the need for detachable mag."
		};
	case slot_function::GUN_DETACHABLE_MAGAZINE:
		return{
			L"Detachable magazine goes here."
		};
	case slot_function::GUN_RAIL:
		return{
			L" "
		};
	case slot_function::GUN_MUZZLE:
		return{
			L"Various gunshot modifiers go here."
		};
	case slot_function::WIELDED_ITEM:
		return{
			L"For holding of almost every item."
		};
	case slot_function::PRIMARY_ARM_BACK:
		return{
			L"Primary arm component."
		};
	case slot_function::SECONDARY_ARM_BACK:
		return{
			L"Secondary arm component."
		};
	case slot_function::TORSO_ARMOR:
		return{
			L"Clothing or armors go here."
		};
	case slot_function::SHOULDER:
		return{
			L"For backpacks or items with belts."
		};
	default: return{ L"Unknown" };
	}
}
