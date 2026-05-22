#pragma once
#include <vector>
#include <cstddef>

#include "game/cosmos/cosmos_solvable_significant.h"
#include "game/common_state/entity_flavours.h"
#include "game/modes/all_mode_includes.h"

#include "application/setups/server/crash_recovery/server_recovery_metadata.h"

/*
	Serializes the full recovery file (magic + header metadata + solvable + mode),
	reusing the snapshot serialization used to send full state to joining clients.
	Runs on the main thread.
*/
std::vector<std::byte> serialize_server_recovery_state(
	const server_recovery_metadata& metadata,
	const all_entity_flavours& flavours,
	const cosmos_solvable_significant& clean_round_state,
	const cosmos_solvable_significant& signi,
	const all_modes_variant& mode
);

/*
	Parses just the header (version + arena + mode + hash) and reports where the body
	starts. Cheap; used at boot to learn which arena/mode to load before the cosmos exists.
*/
bool parse_server_recovery_header(
	const std::vector<std::byte>& bytes,
	server_recovery_metadata& out_metadata,
	std::size_t& out_body_offset
);

/*
	Parses a recovery file. On success fills the out_* arguments and returns true.
	clean_round_state must be the freshly loaded arena's clean state: the
	net_solvable_stream optimization writes only entity ids for static/irrelevant
	entities that are unchanged from clean state, and reads them back from there.
*/
bool parse_server_recovery_state(
	const std::vector<std::byte>& bytes,
	const cosmos_solvable_significant& clean_round_state,
	server_recovery_metadata& out_metadata,
	cosmos_solvable_significant& out_signi,
	all_modes_variant& out_mode
);
