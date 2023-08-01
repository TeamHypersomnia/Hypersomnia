#pragma once
#include "augs/misc/secure_hash.h"
#include "application/setups/server/file_chunk_packet.h"

struct file_download_link_payload {
	address_string_type file_address;
};

struct file_download_payload {
	uint32_t num_file_bytes = 0;
};

struct file_chunks_request_payload {
	augs::constant_size_vector<file_chunk_index_type, 400> requests;
};

struct request_arena_file_download {
	augs::secure_hash_type requested_file_hash;
	file_chunk_index_type num_chunks_to_presend = 0; 
};

