#pragma once
#include "augs/misc/secure_hash.h"

struct file_download_payload {
	std::vector<std::byte> file_bytes;
};

struct request_arena_file_download {
	augs::secure_hash_type requested_file_hash;
};

