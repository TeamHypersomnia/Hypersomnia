#pragma once
#include "application/setups/editor/resources/editor_resource_pools.h"
#include "application/setups/editor/official/official_resource_id_enums.h"

struct editor_filesystem_node;
struct intercosm;

void create_official_resources(const intercosm&, editor_resource_pools& pools);
void create_official_filesystem_from(
	const intercosm&,
	const editor_resource_pools& pools,
	editor_filesystem_node& official_files_root
);
