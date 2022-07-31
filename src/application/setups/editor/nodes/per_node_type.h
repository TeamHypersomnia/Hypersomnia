#pragma once
#include "augs/templates/type_mod_templates.h"
#include "application/setups/editor/nodes/all_editor_node_types_declaration.h"

template <template <class> class Mod>
using per_node_type_container = per_type_container<all_editor_node_types, Mod>;

