#pragma once
#include "augs/readwrite/memory_stream.h"
#include "augs/misc/serialization_buffers.h"
#include "augs/misc/compress.h"

template <bool C>
struct initial_arena_state_payload {
	maybe_const_ref_t<C, cosmos_solvable_significant> signi;
	maybe_const_ref_t<C, online_mode_and_rules> mode;
	maybe_const_ref_t<C, uint32_t> client_id;
};

using ref_net_stream = augs::basic_ref_memory_stream<message_bytes_type>;
using cref_net_stream = augs::basic_ref_memory_stream<const message_bytes_type>;

template <class T, class P>
bool unsafe_read_message(
	T& msg,
	P& payload
) {
	auto s = cref_net_stream(msg.bytes);
	augs::read_bytes(s, payload);

	return true;
}

template <class T, class P>
bool unsafe_write_message(
	T& msg,
	const P& payload
) {
	auto s = ref_net_stream(msg.bytes);
	augs::write_bytes(s, payload);

	return true;
}

namespace net_messages {
	inline bool server_step_entropy::read_payload(server_step_entropy_for_client& output) {
		// TODO SECURITY: don't blindly trust the server!!!
		// TODO BANDWIDTH: optimize entropy i/o

		return unsafe_read_message(*this, output);
	}

	inline bool server_step_entropy::write_payload(const server_step_entropy_for_client& input) {
		return unsafe_write_message(*this, input);
	}

	inline bool client_welcome::read_payload(
		decltype(client_welcome::payload)& output
	) {
		output = std::move(payload);
		return true;
	}

	inline bool client_welcome::write_payload(
		const decltype(client_welcome::payload)& input
	) {
		payload = input;
		return true;
	}

	inline bool new_server_vars::read_payload(
		server_vars& output
	) {
		// TODO SECURITY: don't blindly trust the server!!!
		// TODO BANDWIDTH: optimize vars i/o

		return unsafe_read_message(*this, output);
	}

	inline bool new_server_vars::write_payload(
		const server_vars& input
	) {
		return unsafe_write_message(*this, input);
	}

	inline bool client_entropy::read_payload(
		total_client_entropy& output
	) {
		// TODO SECURITY: don't blindly trust the client!!!
		// TODO BANDWIDTH: optimize entropy i/o

		return unsafe_read_message(*this, output);
	}

	inline bool client_entropy::write_payload(
		const total_client_entropy& input
	) {
		return unsafe_write_message(*this, input);
	}

	inline bool initial_arena_state::read_payload(
		augs::serialization_buffers& buffers,
		const initial_arena_state_payload<false> in
	) {
		const auto data = reinterpret_cast<const std::byte*>(GetBlockData());
		const auto size = static_cast<std::size_t>(GetBlockSize());

		if (size < sizeof(uint32_t)) {
			return false;
		}

		const auto uncompressed_size = *reinterpret_cast<const uint32_t*>(data);
	
		/*
			TODO: validate uncompressed_size with some predefined max solvable size.
		*/

		if (uncompressed_size > 100 * 1024 * 1024) {
			return false;
		}

		buffers.serialization.resize(uncompressed_size);

		augs::decompress(
			data + sizeof(uint32_t),
			size - sizeof(uint32_t),
			buffers.serialization
		);

		auto s = augs::cref_memory_stream(buffers.serialization);

		augs::read_bytes(s, in.signi);
		augs::read_bytes(s, in.mode);
		augs::read_bytes(s, in.client_id);

		return true;
	}

	inline const std::vector<std::byte>* initial_arena_state::write_payload(
		augs::serialization_buffers& buffers,
		const initial_arena_state_payload<true> in
	) {
		auto write_all_to = [&in](auto& s) {
			augs::write_bytes(s, in.signi);
			augs::write_bytes(s, in.mode);
			augs::write_bytes(s, in.client_id);
		};

		{
			augs::byte_counter_stream s;
			write_all_to(s);
			buffers.serialization.reserve(s.size() + sizeof(uint32_t));
		}

		auto s = buffers.make_serialization_stream();

		const auto real_size = static_cast<uint32_t>(buffers.serialization.size());
		augs::write_bytes(s, real_size);

		write_all_to(s);

		augs::compress(buffers.compression_state, buffers.serialization, buffers.compressed);

		return std::addressof(buffers.compressed);
	}
}
