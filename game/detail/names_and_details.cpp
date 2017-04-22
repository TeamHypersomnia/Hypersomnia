#include "entity_description.h"
#include "game/enums/slot_function.h"
#include "game/enums/item_category.h"
#include "augs/ensure.h"

std::wstring get_bbcoded_entity_name(const entity_name n) {
	switch (n) {
	case entity_name::ASSAULT_RIFLE:
		return L"Assault rifle";
	case entity_name::BILMER2000:
		return L"Bilmer 2000";
	case entity_name::KEK9:
		return L"KEK9";
	case entity_name::RED_CHARGE:
		return L"Red charge";
	case entity_name::PINK_CHARGE:
		return L"Pink charge";
	case entity_name::CYAN_CHARGE:
		return L"Cyan charge";
	case entity_name::GREEN_CHARGE:
		return L"Green charge";
	case entity_name::MAGAZINE:
		return L"Magazine";
	case entity_name::TRUCK:
		return L"Truck";
	case entity_name::JMIX114:
		return L"j-Mix114";
	case entity_name::VIOLET_BACKPACK:
		return L"Violet backpack";
	case entity_name::SUPPRESSOR:
		return L"Suppressor";
	case entity_name::SENTIENCE:
		return L"Sentience";
	case entity_name::PERSON:
		return L"Person";
	case entity_name::CRATE:
		return L"Crate";
	case entity_name::PISTOL:
		return L"Cezeta na krotko";
	case entity_name::SUBMACHINE:
		return L"Submachine";
	case entity_name::URBAN_CYAN_MACHETE:
		return L"Urban cyan machete";
	case entity_name::AMPLIFIER_ARM:
		return L"Amplifier arm";
	case entity_name::FORCE_GRENADE:
		return L"Force grenade";
	case entity_name::PED_GRENADE:
		return L"PED grenade";
	case entity_name::INTERFERENCE_GRENADE:
		return L"Force grenade";
	case entity_name::STANDARD_ARM_BACK:
		return L"Standard arm's back";
	case entity_name::STANDARD_ARM_FRONT:
		return L"Standard arm's front";
	default:
		return L"Unknown";
	}
}

std::wstring get_bbcoded_entity_name_details(const entity_name n) {
	switch (n) {
	case entity_name::ASSAULT_RIFLE:
		return{
			L"General purpose charge launcher."
		};
	case entity_name::BILMER2000:
		return{
			L"Robi super naclick."
		};
	case entity_name::KEK9:
		return{
			L"The love to KEK is a fanciful trek."
		};
	case entity_name::RED_CHARGE:
		return{
			L"Deals massive force upon impact.\nBest against armor."
		};
	case entity_name::PINK_CHARGE:
		return{
			L"Effective against both vehicles and personnel."
		};
	case entity_name::CYAN_CHARGE:
		return{
			L"Effective against personnel."
		};
	case entity_name::GREEN_CHARGE:
		return{
			L"Stabilizes functions of body."
		};
	case entity_name::MAGAZINE:
		return{
			L"General purpose charge deposit."
		};
	case entity_name::TRUCK:
		return{
			L"Vehicle for multiple persons.\nCan carry objects\non its huge trailer."
		};
	case entity_name::JMIX114:
		return{
			L"A heavily armored motorcycle.\nSpeedy and agile."
		};
	case entity_name::VIOLET_BACKPACK:
		return{
			L"Can hold items of any kind."
		};
	case entity_name::SUPPRESSOR:
		return{
			L"Limits range at which\ngunshot can be detected.\nReduces recoil and damage."
		};
	case entity_name::SENTIENCE:
		return{
			L"Living entity."
		};
	case entity_name::PERSON:
		return{
			L"Member of Atlantic Nations."
		};
	case entity_name::CRATE:
		return{
			L"Good cover.\nOtherwise useless."
		};
	case entity_name::PISTOL:
		return{
			L"jpr."
		};
	case entity_name::SUBMACHINE:
		return{
			L"Low recoil and damage,\nfast fire rate."
		};
	case entity_name::URBAN_CYAN_MACHETE:
		return{
			L"Delivers solid slash."
		};
	case entity_name::AMPLIFIER_ARM:
		return{
			L"Launches electric missiles\nand gives spells their full potency."
		};
	case entity_name::FORCE_GRENADE:
		return{
			L"Throwable explosive with a three seconds delay.\nDeals damage to [color=red]Health[/color]."
		};
	case entity_name::PED_GRENADE:
		return{
			L"Throwable explosive with a three seconds delay.\nDrains [color=cyan]Personal Electricity[/color].\nIf the subject has [color=turquoise]Electric Shield[/color] enabled,\n the effect is doubled."
		};
	case entity_name::INTERFERENCE_GRENADE:
		return{
			L"Throwable explosive with a three seconds delay.\nDeals damage to [color=orange]Consciousness[/color].\nCauses massive aimpunch."
		};
	case entity_name::STANDARD_ARM_BACK:
		return{
			L"Back of the standard arm."
		};
	case entity_name::STANDARD_ARM_FRONT:
		return{
			L"Front of the standard arm."
		};
	default:
		return{
			L"Unknown"
		};
	}
}

std::wstring get_bbcoded_item_categories(const item_category_bitset& flags) {
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
