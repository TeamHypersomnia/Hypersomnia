#pragma once

enum class network_command {
	ENTROPY_FOR_NEXT_STEP,
	ENTROPY_WITH_HEARTBEAT_FOR_NEXT_STEP,

	COUNT
};

static_assert(static_cast<int>(network_command::COUNT) < 256, "network commands do not fit in one byte!");
