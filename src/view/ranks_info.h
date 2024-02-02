#pragma once
#include "view/necessary_image_id.h"

struct rank_info {
	std::string name;
	assets::necessary_image_id icon;

	float min_mmr;

	rgba name_color;
	rgba number_color;
};

const auto get_rank_info() {
	using I = assets::necessary_image_id;
	const auto bronze = rgba(222, 166, 96, 255);

	return {
		{ "Elo Hell 3",			I::RANK_HELL_3, -99.0f, red, red },
		{ "Elo Hell 2",			I::RANK_HELL_2, -10.0f, red, red },
		{ "Elo Hell 1",			I::RANK_HELL_1, -5.0f, 	red, red },
		{ "Bronze",				I::RANK_1, 		-0.0f,	bronze, green },
		{ "Bronze Elite",		I::RANK_2, 		5.0f,	bronze, green },
		{ "Survivor",			I::RANK_3, 		10.0f, 	bronze, green },
		{ "Seeker",				I::RANK_4, 		15.0f, 	bronze, green },
		{ "Knower",				I::RANK_5, 		20.0f, 	bronze, green },
		{ "Platinum",			I::RANK_6, 		25.0f, 	silver, green },
		{ "Twin Plates",		I::RANK_7, 		30.0f, 	silver, green },
		{ "Starling",			I::RANK_8, 		35.0f, 	silver, green },
		{ "Ascendant",			I::RANK_9,		40.0f, 	silver, green },
		{ "Skylord",			I::RANK_10, 	45.0f, 	silver, silver },
		{ "Aurora Borealis",	I::RANK_11, 	50.0f, 	cyan, cyan },
		{ "Sol Invictus",		I::RANK_12, 	55.0f, 	gold, gold }
	};
};
