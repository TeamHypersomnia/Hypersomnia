#include "augs/filesystem/file.h"

#include "augs/readwrite/memory_stream.h"

#include "game/organization/all_component_includes.h"
#include "game/transcendental/cosmos.h"

#include "augs/readwrite/byte_readwrite.h"

void cosmos_significant_state::clear() {
	*this = cosmos_significant_state();
}