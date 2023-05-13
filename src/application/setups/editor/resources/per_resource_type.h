#pragma once
#include "augs/templates/type_mod_templates.h"
#include "application/setups/editor/resources/all_editor_resource_types_declaration.h"

template <template <class> class Mod>
using per_internal_resource_type_container = per_type_container<internal_editor_resource_types, Mod>;

