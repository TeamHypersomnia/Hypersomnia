#pragma once
#include <vector>
#include "augs/misc/constant_size_string.h"

using steam_rich_presence_key = augs::constant_size_string<64>;
using steam_rich_presence_value = augs::constant_size_string<256>;
using steam_rich_presence_pair = std::pair<steam_rich_presence_key, steam_rich_presence_value>;
using steam_rich_presence_pairs = std::vector<steam_rich_presence_pair>;

