#pragma once
#include "application/setups/server/file_chunk_packet.h"
#include "augs/misc/randomization.h"

struct direct_file_chunk_meta {
	double last_sent = 0.0;
	file_chunk_index_type index;
};

class direct_file_download {
	augs::secure_hash_type current_hash;
	randomization rng;

	uint32_t target_file_size = 0;

	uint32_t num_chunks_downloaded = 0;
	uint32_t num_chunks_total = 0;

	std::vector<std::byte> file_bytes;

	std::vector<direct_file_chunk_meta> chunks;

	uint32_t next_requested_chunk = 0;
	void reshuffle();

public:
	direct_file_download(
		augs::secure_hash_type hash,
		uint32_t num_file_bytes
	);

	std::optional<std::vector<std::byte>> advance(const file_chunk_packet&, uint32_t& data_received);

	direct_file_chunk_meta& request_next_chunk();

	std::size_t get_total_bytes() const {
		return target_file_size;
	}

	std::size_t get_downloaded_bytes() const {
		return std::min(get_total_bytes(), file_chunk_size_v * num_chunks_downloaded);
	}
};

