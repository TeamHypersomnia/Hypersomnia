#pragma once
#include "augs/misc/constant_size_string.h"
#include "application/setups/editor/project/editor_project_meta.h"
#include "application/setups/editor/project/editor_project_about.h"
#include "application/setups/editor/project/editor_layers.h"

#include "application/setups/editor/nodes/editor_node_pools.h"
#include "application/setups/editor/resources/editor_resource_pools.h"

/*
	Note that meta is always the first - 
	so, to identify the game version string of a cached project binary file (and thus determine if the rest is safe to read),
	you will always have to just read the firstmost bytes into game_version_identifier.
*/

struct editor_project {
	// GEN INTROSPECTOR struct editor_project
	editor_project_meta meta;
	editor_project_about about;

	editor_node_pools nodes;
	editor_resource_pools resources;

	editor_layers layers;
	// END GEN INTROSPECTOR
};
