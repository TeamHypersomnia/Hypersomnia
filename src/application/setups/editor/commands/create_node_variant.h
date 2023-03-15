#pragma once
#include "augs/templates/filter_types.h"
#include "application/setups/editor/editor_history_declaration.h"

template <class T>
struct is_create_node_command : std::false_type {};

template <class T>
struct is_create_node_command<create_node_command<T>> : std::true_type {};

using create_node_variant = filter_types_in_list_t<
	is_create_node_command,
	editor_history_base::command_type
>;
