#include "game/detail/sentience_shake.h"
#include "game/components/sentience_component.h"

void sentience_shake::apply(const augs::stepped_timestamp now, components::sentience& sentience) const {
	sentience.time_of_last_shake = now;

	sentience.shake.duration_ms = std::max(duration_ms, sentience.shake.duration_ms);
	sentience.shake.mult = std::max(mult, sentience.shake.mult);
}
