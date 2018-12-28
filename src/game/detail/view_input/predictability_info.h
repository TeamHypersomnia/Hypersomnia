#pragma once
#include "game/cosmos/entity_id.h"

enum class predictability_type {
	ALWAYS,
	NEVER,
	ONLY_BY
};

class prediction_input {
	prediction_input() {}

public:
	bool play_predictable;
	bool play_unpredictable;
	entity_id predicting_subject;

	static auto referential() {
		prediction_input out;
		out.play_predictable = false;
		out.play_unpredictable = true;
		return out;
	}

	static auto predicted() {
		prediction_input out;
		out.play_predictable = true;
		out.play_unpredictable = false;
		return out;
	}

	static auto offline() {
		prediction_input out;
		out.play_predictable = true;
		out.play_unpredictable = true;
		return out;
	}
};

struct predictability_info {
	predictability_type type = predictability_type::ALWAYS;
	entity_id only_by;

	void set_only_by(const entity_id& b) {
		only_by = b;
		type = predictability_type::ONLY_BY;
	}

	bool is_this_predictable(const entity_id& with) const{
		if (type == predictability_type::ALWAYS) {
			return true;
		}

		if (type == predictability_type::ONLY_BY) {
			if (with == only_by) {
				return true;
			}
		}

		return false;
	}

	bool should_play(const prediction_input& in) const {
		const bool predictable = is_this_predictable(in.predicting_subject);

		if (predictable) {
			return in.play_predictable;
		}

		return in.play_unpredictable;
	}
};
