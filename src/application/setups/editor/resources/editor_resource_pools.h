#pragma once
#include <unordered_map>
#include "augs/misc/pool/pool.h"
#include "application/setups/editor/resources/all_editor_resource_types.h"
#include "application/setups/editor/resources/editor_resource_id.h"

#include "augs/misc/pool/pool_helper_templates.h"

template <class T>
using make_editor_resource_pool = augs::pool<T, make_vector, editor_resource_pool_size_type>;
using all_editor_resource_pools = per_type_container<all_editor_resource_types, make_editor_resource_pool>;

struct editor_resource_pools : multipool_dispatchers<editor_resource_pools, editor_resource_id> {
	static constexpr bool json_ignore = true;

	all_editor_resource_pools pools;
	std::unordered_map<editor_resource_id, std::string> names;
};
