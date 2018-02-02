#pragma once
#include "game/transcendental/cosmic_types.h"
#include "game/organization/all_components_declaration.h"
#include "game/transcendental/entity_type.h"

struct entity_guid;
struct child_entity_id;

using cosmic_entity = component_list_t<cosmic_aggregate>;

using unversioned_entity_id = cosmic_object_unversioned_id<cosmic_entity>;
using entity_id = cosmic_object_pool_id<cosmic_entity>;
