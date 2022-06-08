#pragma once
#include "augs/misc/constant_size_string.h"

#include "augs/filesystem/path.h"
#include "augs/filesystem/file_time_type.h"

struct editor_resource_hash {
	// GEN INTROSPECTOR struct editor_resource_hash
	augs::file_time_type source_timestamp;
	augs::path_type source_path;
	std::string blake3_hash;
	// END GEN INTROSPECTOR

	bool operator<(const editor_resource_hash& b) const {
		/* 
			Note: we don't need the natural path order 
			since this operator< will never be used to produce output for humans.

			This is only for deterministic reproduction of the final map hash.
		*/

		return source_path < b.source_path;
	}
};

struct editor_resource_hashes {
	// GEN INTROSPECTOR struct editor_resource_hashes
	std::map<editor_resource_hash> hashes;
	// END GEN INTROSPECTOR
};
