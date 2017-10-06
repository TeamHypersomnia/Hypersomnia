#pragma once
#include "game/organization/all_components_declaration.h"

template <bool is_const>
class basic_entity_handle;

using entity_handle = basic_entity_handle<false>;
using const_entity_handle = basic_entity_handle<true>;