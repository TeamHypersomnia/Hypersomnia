#pragma once
#include "hypersomnia_version.h"
#include "augs/misc/constant_size_string.h"

struct demo_file_meta {
	// GEN INTROSPECTOR struct demo_file_meta
	augs::constant_size_string<256> server_address;
	hypersomnia_version version;
	// END GEN INTROSPECTOR
};
