#pragma once
#include "view/necessary_image_id.h"

struct rank_info {
	std::string name;
	assets::necessary_image_id icon;

	float min_mmr;

	rgba name_color;
	rgba number_color;
};

inline auto get_ranks_info() {
	using I = assets::necessary_image_id;
	const auto bronze = rgba(196, 106, 0, 255);
	const auto bronzer = rgba(222, 166, 96, 255);
	const auto gold = rgba(255, 211, 0, 255);

	return std::array<rank_info, 15> { {
		{ "Elo Hell 3",			I::RANK_HELL_3, -99.0f, red, red },
		{ "Elo Hell 2",			I::RANK_HELL_2, -10.0f, red, red },
		{ "Elo Hell 1",			I::RANK_HELL_1, -5.0f, 	red, red },
		{ "Bronze",				I::RANK_1, 		0.0f,	bronze, green },
		{ "Bronze Elite",		I::RANK_2, 		5.0f,	bronze, green },
		{ "Survivor",			I::RANK_3, 		10.0f, 	bronzer, green },
		{ "Seeker",				I::RANK_4, 		15.0f, 	bronzer, green },
		{ "Knower",				I::RANK_5, 		20.0f, 	bronzer, green },
		{ "Platinum",			I::RANK_6, 		25.0f, 	silver, green },
		{ "Twin Plates",		I::RANK_7, 		30.0f, 	silver, green },
		{ "Starling",			I::RANK_8, 		35.0f, 	white, white },
		{ "Sentinel",			I::RANK_9,		40.0f, 	white, white },
		{ "Skylord",			I::RANK_10, 	45.0f, 	white, white },
		{ "Aurora Borealis",	I::RANK_11, 	50.0f, 	cyan, cyan },
		{ "Sol Invictus",		I::RANK_12, 	55.0f, 	gold, gold }
	} };
};

inline auto get_rank_for(const float mmr) {
	const auto ranks = ::get_ranks_info();
	const auto n = ranks.size();

	for (std::size_t i = 0; i < n; ++i) {
		const auto next = ranks[n - i - 1];

		if (mmr >= next.min_mmr) {
			return next;
		}
	}

	return ranks[0];
}

