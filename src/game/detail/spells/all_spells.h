#pragma once
#include "augs/templates/transform_types.h"
#include "augs/templates/instance_type.h"
#include "augs/templates/type_in_list_id.h"

#include "augs/misc/trivially_copyable_tuple.h"

#include "game/detail/spells/haste.h"
#include "game/detail/spells/fury_of_the_aeons.h"
#include "game/detail/spells/ultimate_wrath_of_the_aeons.h"
#include "game/detail/spells/electric_shield.h"
#include "game/detail/spells/electric_triad.h"
#include "game/detail/spells/exaltation.h"
#include "game/detail/spells/echoes_of_the_higher_realms.h"

template <template <class...> class List>
using spell_list_t = List<
	haste,
	fury_of_the_aeons,
	ultimate_wrath_of_the_aeons,
	electric_shield,
	electric_triad,
	exaltation,
	echoes_of_the_higher_realms
>;

template <template <class...> class List>
using spell_instance_list_t = transform_types_in_list_t<
	spell_list_t<List>,
	instance_of
>;

using spell_instance_tuple = spell_instance_list_t<augs::trivially_copyable_tuple>;
using spell_id = type_in_list_id<spell_list_t<type_list>>;

template <class T>
constexpr bool is_spell_v = is_one_of_list_v<remove_cref<T>, spell_list_t<type_list>>;
	
using learnt_spells_array_type = std::array<
	bool,
	aligned_num_of_bytes_v<num_types_in_list_v<spell_instance_tuple>, 4>
>;
