#pragma once
#include "augs/templates/transform_types.h"
#include "augs/templates/instance_type.h"
#include "augs/templates/type_in_list_id.h"

#include "game/detail/spells/haste.h"
#include "game/detail/spells/fury_of_the_aeons.h"
#include "game/detail/spells/ultimate_wrath_of_the_aeons.h"
#include "game/detail/spells/electric_shield.h"
#include "game/detail/spells/electric_triad.h"

template <template <typename...> class List>
struct put_all_spells_into {
	using type = List<
		haste,
		fury_of_the_aeons,
		ultimate_wrath_of_the_aeons,
		electric_shield,
		electric_triad
	>;
};

template <template <typename...> class List>
using put_all_spells_into_t = typename put_all_spells_into<List>::type;

template <template <typename...> class List>
using put_all_spell_instances_into_t = transform_types_in_list_t<
	put_all_spells_into_t<List>,
	make_instance
>;

using spell_instance_tuple = put_all_spell_instances_into_t<augs::trivially_copyable_tuple>;
using spell_id = type_in_list_id<spell_instance_tuple>;