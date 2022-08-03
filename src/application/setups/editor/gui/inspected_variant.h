#pragma once
#include "application/setups/editor/nodes/editor_node_id.h"
#include "application/setups/editor/resources/editor_resource_id.h"
#include "application/setups/editor/project/editor_layer_id.h"

using inspected_variant = std::variant<
	editor_node_id,
	editor_resource_id,
	editor_layer_id
>;
