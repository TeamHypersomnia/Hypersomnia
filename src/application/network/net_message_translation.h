#pragma once
#include "augs/readwrite/memory_stream.h"
#include "augs/misc/serialization_buffers.h"
#include "augs/misc/compress.h"
#include "augs/misc/readable_bytesize.h"
#include "augs/templates/logically_empty.h"
#include "application/network/net_serialize.h"
#include "application/network/net_solvable_stream.h"
#include "augs/string/get_type_name.h"

template <bool C>
struct full_arena_snapshot_payload {
	maybe_const_ref_t<C, cosmos_solvable_significant> signi;
	maybe_const_ref_t<C, all_modes_variant> mode;
	maybe_const_ref_t<C, uint32_t> client_id;
	maybe_const_ref_t<C, rcon_level_type> rcon;
};

template <class T, class P>
bool unsafe_read_message(
	T& msg,
	P& payload
) {
	auto s = augs::basic_ref_memory_stream<const decltype(msg.bytes)>(msg.bytes);
	augs::read_bytes(s, payload);

	return true;
}

template <class T, class P>
bool unsafe_write_message(
	T& msg,
	const P& payload
) {
	auto s = augs::basic_ref_memory_stream<decltype(msg.bytes)>(msg.bytes);
	augs::write_bytes(s, payload);

	return true;
}

template <class F, class T>
bool write_standard_block_message(
	F block_allocator,
	const T& input
) {
	augs::byte_counter_stream s;
	augs::write_bytes(s, input);

	auto block = block_allocator(s.size());
	auto pts = augs::make_ptr_write_stream(reinterpret_cast<std::byte*>(block), s.size());

	augs::write_bytes(pts, input);

	return true;
}

template <typename T>
bool only_block_message::read_standard_block_message(T& output) {
	auto data = reinterpret_cast<const std::byte*>(GetBlockData());
	auto size = static_cast<std::size_t>(GetBlockSize());

	auto pts = augs::make_ptr_read_stream(data, size);

	try {
		augs::read_bytes(pts, output);
	}
	catch (const augs::stream_read_error& err) {
		LOG("Failed to read %x: %x", get_type_name<T>(), err.what());
		return false;
	}

	return true;
}

constexpr std::size_t max_server_step_size_v = 
	max_message_size_v 
	- yojimbo::ConservativeMessageHeaderBits / 8
;

namespace net_messages {
	inline bool new_server_vars::read_payload(
		server_vars& output
	) {
		const auto result = read_standard_block_message(output);

		if (!sanitization::arena_name_safe(output.arena)) {
			return false;
		}

		return result;
	}

	template <class F>
	inline bool new_server_vars::write_payload(
		F block_allocator,
		const server_vars& input
	) {
		return write_standard_block_message(block_allocator, input);
	}

	inline bool new_server_public_vars::read_payload(
		server_public_vars& output
	) {
		// TODO SECURITY: don't blindly trust the server!!!
		// TODO BANDWIDTH: optimize vars i/o

		try {
			return unsafe_read_message(*this, output);
		}
		catch (const augs::stream_read_error& err) {
			LOG_NVPS("Failed to read new_server_public_vars: %x", err.what());
			return false;
		}
	}

	inline bool new_server_public_vars::write_payload(
		const server_public_vars& input
	) {
		return unsafe_write_message(*this, input);
	}

	template <class F>
	inline bool new_server_runtime_info::write_payload(
		F block_allocator,
		const server_runtime_info& input
	) {
		return write_standard_block_message(block_allocator, input);
	}

	inline bool new_server_runtime_info::read_payload(
		server_runtime_info& output
	) {
		return read_standard_block_message(output);
	}

	inline bool auth_request::read_payload(
		auth_request_payload& payload
	) {
		const auto data = reinterpret_cast<const std::byte*>(GetBlockData());
		const auto size = static_cast<std::size_t>(GetBlockSize());

		if (size < 1) {
			return false;
		}

		auto pts = augs::make_ptr_read_stream(data, size);

		{
			uint8_t type;
			augs::read_bytes(pts, type);
			payload.type = static_cast<auth_provider_type>(type);
		}

		auto& ticket = payload.ticket_bytes;
		ticket.resize(size - 1);

		augs::detail::read_raw_bytes(pts, ticket.data(), ticket.size());

		return true;
	}

	template <class F>
	inline bool auth_request::write_payload(
		F block_allocator,
		const auth_request_payload& payload
	) {
		const auto& ticket = payload.ticket_bytes;
		const auto sz = ticket.size() + 1;
		const auto block = reinterpret_cast<std::byte*>(block_allocator(sz));
		auto pts = augs::make_ptr_write_stream(block, sz);

		augs::write_bytes(pts, static_cast<uint8_t>(payload.type));
		augs::detail::write_raw_bytes(pts, ticket.data(), ticket.size());

		return true;
	}

	inline bool player_avatar_exchange::read_payload(
		mode_player_id& player_id,
		arena_player_avatar_payload& payload
	) {
		using id_type = mode_player_id::id_value_type;

		auto data = reinterpret_cast<const std::byte*>(GetBlockData());
		auto size = static_cast<std::size_t>(GetBlockSize());

		const bool player_id_written_properly = size >= sizeof(id_type);

		if (!player_id_written_properly) {
			return false;
		}

		player_id.value = *reinterpret_cast<const id_type*>(data);

		data += sizeof(id_type);
		size -= sizeof(id_type);

		if (size > max_avatar_bytes_v) {
			return false;
		}

		payload.image_bytes.resize(size);
		std::memcpy(payload.image_bytes.data(), data, size);

		return true;
	}

	template <class F>
	inline bool player_avatar_exchange::write_payload(
		F block_allocator,
		const mode_player_id& player_id,
		const arena_player_avatar_payload& payload
	) {
		using id_type = mode_player_id::id_value_type;

		const auto& png = payload.image_bytes;
		auto block = block_allocator(sizeof(id_type) + png.size());

		std::memcpy(block, &player_id.value, sizeof(player_id));
		std::memcpy(block + sizeof(player_id), png.data(), png.size());

		return true;
	}

	inline bool full_arena_snapshot::read_payload(
		augs::serialization_buffers& buffers,
		const cosmos_solvable_significant& clean_round_state,
		const full_arena_snapshot_payload<false> in
	) {
		const auto data = reinterpret_cast<const std::byte*>(GetBlockData());
		const auto size = static_cast<std::size_t>(GetBlockSize());

		NSR_LOG("RECEIVING INITIAL STATE");

		LOG("Compressed arena snapshot size: %x", size);

		const bool size_written_properly = size >= sizeof(uint32_t);

		if (!size_written_properly) {
			return false;
		}

		const auto uncompressed_size = *reinterpret_cast<const uint32_t*>(data);
	
		LOG("Uncompressed arena snapshot size: %x", uncompressed_size);

		/*
			TODO: validate uncompressed_size with some predefined max solvable size.
		*/

		if (uncompressed_size > 100 * 1024 * 1024) {
			return false;
		}

		auto& uncompressed_buf = buffers.serialization;
		uncompressed_buf.resize(uncompressed_size);

		try {
			augs::decompress(
				data + sizeof(uint32_t),
				size - sizeof(uint32_t),
				uncompressed_buf
			);

			NSR_LOG("Successfully uncompressed the initial state.");
		}
		catch (const augs::decompression_error& err) {
			LOG("Failed to decompress the initial state. Server might be malicious.");
			LOG(err.what());

			return false;
		}

		auto s = net_solvable_stream_cref(clean_round_state, uncompressed_buf);

		augs::read_bytes(s, in.signi);
		augs::read_bytes(s, in.mode);
		augs::read_bytes(s, in.client_id);
		augs::read_bytes(s, in.rcon);

		NSR_LOG_NVPS(in.client_id);

		return true;
	}

	template <class F>
	inline bool full_arena_snapshot::write_payload(
		F block_allocator,
		augs::serialization_buffers& buffers,
		const cosmos_solvable_significant& clean_round_state,
		const all_entity_flavours& all_flavours,
		const full_arena_snapshot_payload<true> in
	) {
		auto write_all_to = [&in](auto& s) {
			augs::write_bytes(s, in.signi);
			augs::write_bytes(s, in.mode);
			augs::write_bytes(s, in.client_id);
			augs::write_bytes(s, in.rcon);
		};

		NSR_LOG("SENDING INITIAL STATE");

		{
			NSR_LOG("STAGE: ESTIMATION");

			augs::byte_counter_stream s;
			write_all_to(s);
			buffers.serialization.reserve(s.size());

			NSR_LOG("Reserved size: %x", s.size());

			{
				auto s = buffers.make_serialization_stream<net_solvable_stream_ref>(all_flavours, clean_round_state, in.signi);
				write_all_to(s);
			}

			NSR_LOG("Result stream length: %x", buffers.serialization.size());
		}

		auto& c = buffers.compressed;

		{
			NSR_LOG("STAGE: COMPRESSION");

			c.clear();

			{
				auto s = augs::ref_memory_stream(c);
				const auto uncompressed_size = static_cast<uint32_t>(buffers.serialization.size());
				augs::write_bytes(s, uncompressed_size);

				LOG("Uncompressed arena snapshot size: %x", uncompressed_size);
			}

			augs::compress(buffers.compression_state, buffers.serialization, c);

			LOG("Compressed arena snapshot size: %x", c.size());
		}

		auto block = block_allocator(c.size());
		std::memcpy(block, c.data(), c.size());

		return true;
	}
}
