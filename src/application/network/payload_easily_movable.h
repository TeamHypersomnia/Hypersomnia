#pragma once

template <class T>
constexpr bool payload_easily_movable_v = !is_one_of_v<
	T,
	full_arena_snapshot_payload<false>,
	arena_player_avatar_payload
>;

