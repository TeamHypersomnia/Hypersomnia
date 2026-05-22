#include <array>
#include <string>
#include <vector>

/*
	Include the full cosmos and mode types first (which pull in the entity
	invariants/flavours net_solvable_stream needs), plus the augs readwrite
	machinery. We intentionally avoid net_serialize.h / yojimbo here so that
	yojimbo's write_bytes/read_bytes macros do not shadow augs::write_bytes.
*/

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

/*
	pool_io.hpp provides the read/write template body for augs::pool. In a HEADLESS
	build the client_setup TU (the usual instantiator of the net_solvable_stream_cref
	read path) is not compiled, so without including it here the entity-pool read
	instantiations would fail to link.
*/
#include "augs/misc/pool/pool_io.hpp"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/cosmos_solvable_significant.h"
#include "game/common_state/entity_flavours.h"
#include "game/modes/all_mode_includes.h"

#include "application/network/net_solvable_stream.h"

#include "application/setups/server/crash_recovery/server_recovery_serialize.h"

static constexpr std::array<char, 8> server_recovery_magic_v =
	{{'H','Y','P','S','R','C','O','V'}}
;

std::vector<std::byte> serialize_server_recovery_state(
	const server_recovery_metadata& metadata,
	const all_entity_flavours& flavours,
	const cosmos_solvable_significant& clean_round_state,
	const cosmos_solvable_significant& signi,
	const all_modes_variant& mode
) {
	auto body = std::vector<std::byte>();

	{
		auto s = net_solvable_stream_ref(flavours, clean_round_state, signi, body);

		augs::write_bytes(s, signi);
		augs::write_bytes(s, mode);
	}

	auto file_bytes = std::vector<std::byte>();

	{
		auto s = augs::ref_memory_stream(file_bytes);

		augs::write_bytes(s, server_recovery_magic_v);
		augs::write_bytes(s, metadata.version);
		augs::write_bytes(s, metadata.arena_name);
		augs::write_bytes(s, metadata.game_mode);
		augs::write_bytes(s, metadata.arena_hash);

		s.write(body.data(), body.size());
	}

	return file_bytes;
}

bool parse_server_recovery_header(
	const std::vector<std::byte>& bytes,
	server_recovery_metadata& out_metadata,
	std::size_t& out_body_offset
) {
	try {
		auto s = augs::cref_memory_stream(bytes);

		auto magic = std::array<char, 8>();
		augs::read_bytes(s, magic);

		if (magic != server_recovery_magic_v) {
			return false;
		}

		augs::read_bytes(s, out_metadata.version);
		augs::read_bytes(s, out_metadata.arena_name);
		augs::read_bytes(s, out_metadata.game_mode);
		augs::read_bytes(s, out_metadata.arena_hash);

		out_body_offset = s.get_read_pos();
	}
	catch (...) {
		return false;
	}

	return out_body_offset <= bytes.size();
}

bool parse_server_recovery_state(
	const std::vector<std::byte>& bytes,
	const cosmos_solvable_significant& clean_round_state,
	server_recovery_metadata& out_metadata,
	cosmos_solvable_significant& out_signi,
	all_modes_variant& out_mode
) {
	std::size_t body_offset = 0;

	if (!::parse_server_recovery_header(bytes, out_metadata, body_offset)) {
		return false;
	}

	try {
		const auto body = std::vector<std::byte>(bytes.begin() + body_offset, bytes.end());

		auto s = net_solvable_stream_cref(clean_round_state, body);

		augs::read_bytes(s, out_signi);
		augs::read_bytes(s, out_mode);
	}
	catch (...) {
		return false;
	}

	return true;
}
