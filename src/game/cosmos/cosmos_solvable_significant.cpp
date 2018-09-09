#include "augs/filesystem/file.h"

#include "augs/readwrite/memory_stream.h"

#include "game/organization/all_component_includes.h"
#include "game/cosmos/cosmos.h"

void cosmos_solvable_significant::clear() {
	*this = cosmos_solvable_significant();
	global.clear();
}