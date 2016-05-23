#pragma once

enum party_category {
	METROPOLIS_CITIZEN = 1 << 0,
	ATLANTIS_CITIZEN = 1 << 1,
	RESISTANCE_CITIZEN = 1 << 2
};

enum class attitude_type {
	NEUTRAL,
	WANTS_TO_HEAL,
	WANTS_TO_KILL,
	WANTS_TO_KNOCK_UNCONSCIOUS
};