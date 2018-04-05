#pragma once
#include "hypersomnia_version.h"
#include "augs/filesystem/path.h"
#include "augs/templates/for_each_std_get.h"
#include "augs/templates/string_templates.h"

#include "augs/misc/typesafe_sprintf.h"

#include "game/organization/for_each_entity_type.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"
#include "augs/misc/readable_bytesize.h"

inline auto static_allocations_info() {
	return typesafe_sprintf(
		"STATICALLY_ALLOCATE_ENTITIES=%x\n"
		"STATICALLY_ALLOCATE_ASSETS=%x\n"
		"STATICALLY_ALLOCATE_BAKED_FONTS=%x\n",
		STATICALLY_ALLOCATE_ENTITIES,
		STATICALLY_ALLOCATE_ASSETS,
		STATICALLY_ALLOCATE_BAKED_FONTS
	);
}

inline auto sizeofs_info() {
	return typesafe_sprintf(
		"cosmos=%x, entity_id=%x, unversioned=%x\n",
		readable_bytesize(sizeof(cosmos)), sizeof(entity_id), sizeof(unversioned_entity_id)
	);
}

inline auto entity_types_info() {
	std::string output;

	for_each_entity_type([&](auto e){
		output += get_type_name_strip_namespace(e) + '\n';
	});

	return output;
}

inline auto complete_build_info() {
	return 
		hypersomnia_version().get_summary()
		+ "\nCompilation flags:\n"
		+ static_allocations_info()
		+ "\nOther:\n"
		+ sizeofs_info()
		+ "\nAll entity types:\n"
		+ entity_types_info()
	;
}

void dump_detailed_sizeof_information(const augs::path_type& where);
