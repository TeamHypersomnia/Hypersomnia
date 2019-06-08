#include "game/detail/sentience_shake.h"
#include "game/components/sentience_component.h"

void sentience_shake::apply(
	const augs::stepped_timestamp now,
	const invariants::sentience& sentience_def,
	components::sentience& sentience
) const {
	if (augs::is_zero(mult)) {
		return;
	}

	const auto& s = sentience_def.shake_settings;

	sentience.time_of_last_shake = now;

	sentience.shake.duration_ms = std::clamp(duration_ms * s.duration_mult, sentience.shake.duration_ms, s.max_duration_ms);
	//sentience.shake.mult = std::clamp(mult * s.mult, sentience.shake.mult, sentience_def.shake_settings.max_mult);
}
