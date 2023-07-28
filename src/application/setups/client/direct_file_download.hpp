#pragma once
#include "application/setups/client/direct_file_download.h"

direct_file_download::direct_file_download(
	augs::secure_hash_type hash,
	uint32_t num_file_bytes
) : current_hash(hash), target_file_size(num_file_bytes) {
	ensure(num_file_bytes < max_direct_download_file_size_v);
	ensure(num_file_bytes > 0);

	num_chunks_total = num_file_bytes / file_chunk_size_v;

	if (num_file_bytes % file_chunk_size_v != 0) {
		++num_chunks_total;
	}

	chunks.resize(num_chunks_total);

	for (uint32_t i = 0; i < num_chunks_total; ++i) {
		chunks[i].index = i;
	}

	file_bytes.resize(num_chunks_total * file_chunk_size_v);
}

void direct_file_download::reshuffle() {
	shuffle_range(chunks, rng);
}

direct_file_chunk_meta& direct_file_download::request_next_chunk() {
	if (next_requested_chunk >= chunks.size()) {
		next_requested_chunk = 0;
		reshuffle();
	}

	return chunks[next_requested_chunk++];
} 

std::optional<std::vector<std::byte>> direct_file_download::advance(const file_chunk_packet& payload, uint32_t& data_received) {
	data_received = 0;

	if (payload.file_hash != current_hash) {
		// LOG("Wrong hash.");
		return std::nullopt;
	}

	bool found_index = false;

	erase_if(chunks, [&](const auto& candidate) {
		if (candidate.index == payload.index) {
			const auto& real_index = index_in(chunks, candidate);

			if (real_index < next_requested_chunk) {
				--next_requested_chunk;
			}

			found_index = true;
			return true;
		}

		return false;
	});

	if (!found_index) {
		// LOG("Received %x but we already have it.", payload.index);

		return std::nullopt;
	}
	else {
		// LOG("Received %x and it's new.", payload.index);
	}

	data_received = file_chunk_size_v;
	++num_chunks_downloaded;

	std::memcpy(
		file_bytes.data() + payload.index * file_chunk_size_v,
		payload.chunk_bytes.data(),
		file_chunk_size_v
	);

	if (chunks.empty()) {
		file_bytes.resize(target_file_size);
		return std::move(file_bytes);
	}

	return std::nullopt;
}

