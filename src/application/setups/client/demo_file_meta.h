#pragma once
#include <cstdint>
#include "hypersomnia_version.h"
#include "augs/misc/constant_size_string.h"
#include "augs/network/network_types.h"

struct demo_file_meta {
	// GEN INTROSPECTOR struct demo_file_meta
	server_name_type server_name;
	address_string_type server_address;
	hypersomnia_version version;
	version_timestamp_string when_recorded;
	int64_t uncompressed_size = 0;
	// END GEN INTROSPECTOR
};
