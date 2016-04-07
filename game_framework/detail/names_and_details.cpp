#include "entity_description.h"

entity_description description_by_entity_name(entity_name n) {
	switch (n) {
	case entity_name::ASSAULT_RIFLE:
		return{
			L"Assault rifle",
			L"General purpose charge launcher."
		};
	case entity_name::PINK_CHARGE:
		return{
			L"Pink charge",
			L"Bullet charge.\nEffective against vehicles."
		};
	case entity_name::CYAN_CHARGE:
		return{
			L"Pink charge",
			L"Bullet charge.\nEffective against personnel."
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
			L"Magazine",
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
	default:
		return{
			L"Unknown",
			L"Unknown"
		};
	}
}