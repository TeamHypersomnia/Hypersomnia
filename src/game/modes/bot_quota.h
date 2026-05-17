#pragma once
#include <cstdint>
#include "augs/misc/constant_size_vector.h"

/*
	bot_quota encodes how many bots should be spawned.

	empty()   -> use the map default (rules.default_bot_quota)
	size 1    -> total number of bots, split per team
	size 2    -> per team: [own_side, opposing_side] (when a requester is known)

	Mirrors chat: /bots N -> {N}, /bots N M -> {N, M}.
*/
using bot_quota = augs::constant_size_vector<uint8_t, 2>;
