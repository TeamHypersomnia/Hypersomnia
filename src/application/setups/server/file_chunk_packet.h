#pragma once

#define NETCODE_AUXILIARY_COMMAND_PACKET 200

using file_chunk_index_type = uint16_t;
constexpr std::size_t file_chunk_packet_size_v = 1000;
constexpr std::size_t file_chunk_meta_size_v = 2 * sizeof(uint8_t) + sizeof(file_chunk_index_type) + sizeof(augs::secure_hash_type);
constexpr std::size_t file_chunk_size_v = file_chunk_packet_size_v - file_chunk_meta_size_v;
using file_chunk_bytes_type = std::array<std::byte, file_chunk_size_v>;

constexpr std::size_t max_direct_download_file_size_v = std::numeric_limits<file_chunk_index_type>::max() * file_chunk_size_v;

struct file_chunk_packet {
	uint8_t command = NETCODE_AUXILIARY_COMMAND_PACKET;
	uint8_t pad = 0;
	file_chunk_index_type index = 0;
	augs::secure_hash_type file_hash = {};
	file_chunk_bytes_type chunk_bytes = {};

	bool header_valid() const {
		return command == NETCODE_AUXILIARY_COMMAND_PACKET;
	}
};

static_assert(std::is_trivially_copyable_v<file_chunk_packet>);
static_assert(sizeof(file_chunk_packet) == file_chunk_packet_size_v);
static_assert(sizeof(file_chunk_packet) == 
	file_chunk_meta_size_v + file_chunk_size_v
);
