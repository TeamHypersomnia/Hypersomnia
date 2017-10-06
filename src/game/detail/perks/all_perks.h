#pragma once
#include "augs/templates/transform_types.h"
#include "augs/templates/instance_type.h"
#include "augs/templates/type_in_list_id.h"

#include "game/detail/perks/haste_perk.h"
#include "game/detail/perks/electric_shield_perk.h"

template <template <class...> class List>
using perk_list_t = List<
	haste_perk,
	electric_shield_perk
>;

template <template <class...> class List>
using perk_instance_list_t = transform_types_in_list_t<
	perk_list_t<List>,
	instance_of
>;

using perk_instance_tuple = perk_instance_list_t<augs::trivially_copyable_tuple>;
using perk_id = type_in_list_id<perk_instance_tuple>;