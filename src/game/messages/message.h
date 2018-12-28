#pragma once
#include "game/cosmos/entity_id.h"
#include "game/detail/view_input/predictability_info.h"

namespace messages {
	struct message {
		entity_id subject;

		message(const entity_id subject = {}) : subject(subject) {}
	};

	struct predicted_message {
		predictability_info predictability;

		predicted_message(
			const predictability_info& predictability = never_predictable_v
		) noexcept : 
			predictability(predictability) 
		{}

		predicted_message(const predicted_message&) noexcept = default;
		predicted_message(predicted_message&&) noexcept = default;

		predicted_message& operator=(const predicted_message&) noexcept = default;
		predicted_message& operator=(predicted_message&&) noexcept = default;

		const auto& get_predictability() const {
			return predictability;
		}
	};
}