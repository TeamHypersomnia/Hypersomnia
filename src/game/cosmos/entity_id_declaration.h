#pragma once
#include "augs/misc/pool/pool_declaration.h"
#include "game/cosmos/pool_size_type.h"

using cosmic_pool_undo_free_input = augs::pool_undo_free_input<cosmic_pool_size_type>;

using cosmic_object_pool_id = augs::pooled_object_id<cosmic_pool_size_type>;
using cosmic_object_unversioned_id = augs::unversioned_id<cosmic_pool_size_type>;

using unversioned_entity_id_base = cosmic_object_unversioned_id;
using entity_id_base = cosmic_object_pool_id;

struct entity_id;
struct unversioned_entity_id;
struct child_entity_id;

template <class E>
struct typed_entity_id;

using signi_entity_id = entity_id;
