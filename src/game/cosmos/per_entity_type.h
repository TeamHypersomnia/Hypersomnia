#pragma once
#include <tuple>
#include <array>

#include "augs/templates/per_type.h"
#include "game/organization/all_entity_types_declaration.h"

template <template <class> class Mod>
using per_entity_type = per_type_t<all_entity_types, Mod>;

template <template <class> class Mod>
using per_entity_type_container = per_type_container<all_entity_types, Mod>;

template <class T>
using per_entity_type_array = std::array<T, num_types_in_list_v<all_entity_types>>;
