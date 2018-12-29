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

	static auto referential(const entity_id with) {
		prediction_input out;
		out.play_predictable = false;
		out.play_unpredictable = true;
		out.predicting_subject = with;
		return out;
	}

	static auto predicted(const entity_id with) {
		prediction_input out;
		out.play_predictable = true;
		out.play_unpredictable = false;
		out.predicting_subject = with;
		return out;
	}

	static auto offline() {
		prediction_input out;
		out.play_predictable = true;
		out.play_unpredictable = true;
		return out;
	}
};

struct effect_prediction_settings {
	// GEN INTROSPECTOR struct effect_prediction_settings
	bool predict_death_particles = false;
	bool predict_death_sounds = false;
	// END GEN INTROSPECTOR
};

struct predictability_info {
	predictability_type type = predictability_type::ALWAYS;
	entity_id predicting_subject;

	void set_only_by(const entity_id& b) {
		type = predictability_type::ONLY_BY;
		predicting_subject = b;
	}

	void set_never() {
		type = predictability_type::NEVER;
		predicting_subject = {};
	}

	void set_always() {
		type = predictability_type::ALWAYS;
		predicting_subject = {};
	}

	static auto never() {
		predictability_info out;
		out.set_never();
		return out;
	}

	static auto always() {
		predictability_info out;
		out.set_always();
		return out;
	}

	static auto only_by(const entity_id& b) {
		predictability_info out;
		out.set_only_by(b);
		return out;
	}

	bool is_this_predictable(const entity_id& with) const{
		if (type == predictability_type::ALWAYS) {
			return true;
		}

		if (type == predictability_type::ONLY_BY) {
			if (with == predicting_subject) {
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

#define never_predictable_v predictability_info::never()
#define always_predictable_v predictability_info::always()
#define predictable_only_by(x) predictability_info::only_by(x)
