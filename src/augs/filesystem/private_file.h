#pragma once
#include <string>
#include "augs/filesystem/path_declaration.h"

namespace augs {
	/*
		A new file readable by its owner only from the start (never one that exists - false then), its content flushed
		to the disk before this returns true. For secrets (keys, passwords): written under a temporary name, then put
		in place.
	*/

	bool write_new_private_file(const path_type& path, const std::string& content);
}
