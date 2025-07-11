#pragma once
#include <cstdint>
#include <unordered_map>
#include "augs/misc/pool/pool.h"
#include "application/setups/editor/nodes/all_editor_node_types.h"
#include "application/setups/editor/nodes/editor_node_id.h"
#include "application/setups/editor/nodes/editor_typed_node_id.h"

#include "augs/misc/pool/pool_helper_templates.h"

template <class T>
using make_editor_node_pool = augs::pool<T, make_vector, editor_node_pool_size_type>;
using all_editor_node_pools = per_type_container<all_editor_node_types, make_editor_node_pool>;

struct editor_node_pools : multipool_dispatchers<editor_node_pools, editor_node_id, editor_typed_node_id> {
	static constexpr bool json_ignore = true;

	all_editor_node_pools pools;
	uint32_t next_chronological_order = 0;
};
