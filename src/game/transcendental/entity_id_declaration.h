#pragma once
#include "game/organization/all_components_declaration.h"

#include "game/transcendental/cosmic_types.h"
#include "game/transcendental/entity_type.h"

using cosmic_entity = component_list_t<cosmic_aggregate>;

using unversioned_entity_id_base = cosmic_object_unversioned_id<cosmic_entity>;
using entity_id_base = cosmic_object_pool_id<cosmic_entity>;

struct entity_guid;

struct entity_id;
struct unversioned_entity_id;
struct child_entity_id;
