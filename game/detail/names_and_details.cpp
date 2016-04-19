#include "entity_description.h"
#include "../globals/inventory.h"
#include "ensure.h"

textual_description description_by_entity_name(entity_name n) {
	switch (n) {
	case entity_name::ASSAULT_RIFLE:
		return{
			L"Assault rifle",
			L"General purpose charge launcher."
		};
	case entity_name::PINK_CHARGE:
		return{
			L"Pink charge",
			L"Effective against vehicles."
		};
	case entity_name::CYAN_CHARGE:
		return{
			L"Cyan charge",
			L"Effective against personnel."
		};
	case entity_name::MAGAZINE:
		return{
			L"Magazine",
			L"General purpose charge deposit."
		};
	case entity_name::TRUCK:
		return{
			L"Truck",
			L"Vehicle for multiple persons.\nCan carry objects\non its huge trailer."
		};
	case entity_name::MOTORCYCLE:
		return{
			L"Motorcycle",
			L"Vehicle for one person.\nSpeedy and agile."
		};
	case entity_name::VIOLET_BACKPACK:
		return{
			L"Violet backpack",
			L"Can hold items of any kind."
		};
	case entity_name::SUPPRESSOR:
		return{
			L"Suppressor",
			L"Limits range at which\ngunshot can be detected.\nReduces recoil and damage."
		};
	case entity_name::SENTIENCE:
		return{
			L"Sentience",
			L"Living entity."
		};
	case entity_name::PERSON:
		return{
			L"Person",
			L"Living entity."
		};
	case entity_name::CRATE:
		return{
			L"Crate",
			L"Good cover.\nOtherwise useless."
		};
	case entity_name::PISTOL:
		return{
			L"Cezeta na krotko",
			L"jpr."
		};
	case entity_name::SUBMACHINE:
		return{
			L"Submachine",
			L"Low recoil and damage,\nfast fire rate."
		};
	case entity_name::URBAN_CYAN_MACHETE:
		return{
			L"Urban cyan machete",
			L"Delivers solid slash."
		};
	default:
		ensure(false);
		return{
			L"Unknown",
			L"Unknown"
		};
	}
}

std::wstring describe_item_compatibility_categories(unsigned flags) {
	std::wstring result;

	if (flags & item_category::MAGAZINE) 
		result += L"Magazine, ";
	if (flags & item_category::BARREL_ATTACHMENT) 
		result += L"Barrel attachment, ";
	if (flags & item_category::RAIL_ATTACHMENT) 
		result += L"Rail attachment, ";
	if (flags & item_category::SHOT_CHARGE) 
		result += L"Shot charge, ";
	if (flags & item_category::SHOULDER_CONTAINER) 
		result += L"Shoulder container, ";
	if (flags & item_category::TORSO_ARMOR) 
		result += L"Torso armor, ";

	if (!result.empty())
		result = result.substr(0, result.length() - 2);
	else
		result = L"Everything";

	return result;
}

textual_description describe_slot_function(slot_function f) {
	switch (f) {
	case slot_function::GUN_CHAMBER:
		return{
			L"Chamber",
			L"Single charge is inserted here\nprior to being fired."
		};
	case slot_function::GUN_CHAMBER_MAGAZINE:
		return{
			L"Integral magazine",
			L"Internal storage for charges\nremoving the need for detachable mag."
		};
	case slot_function::GUN_DETACHABLE_MAGAZINE:
		return{
			L"Magazine slot",
			L"Detachable magazine goes here."
		};
	case slot_function::GUN_RAIL:
		return{
			L"Rail",
			L" "
		};
	case slot_function::GUN_BARREL:
		return{
			L"Barrel",
			L"Various gunshot modifiers go here."
		};
	case slot_function::PRIMARY_HAND:
		return{
			L"Primary hand",
			L"Can hold almost every item."
		};
	case slot_function::SECONDARY_HAND:
		return{
			L"Secondary hand",
			L"Can hold almost every item."
		};
	case slot_function::TORSO_ARMOR_SLOT:
		return{
			L"Torso",
			L"Clothing or armors go here."
		};
	case slot_function::SHOULDER_SLOT:
		return{
			L"Shoulder",
			L"For backpacks or items with belts."
		};
	default: ensure(false);
	}
}
