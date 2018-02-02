#pragma once
#include <unordered_map>
#include "game/transcendental/entity_id.h"

template <class cache_type>
using audiovisual_cache_map = std::unordered_map<unversioned_entity_id, cache_type>;

