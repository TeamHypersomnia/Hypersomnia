#pragma once

/*
	Default/idle behavior.
	Bot is doing nothing and waiting to be assigned a task.
*/

struct ai_behavior_idle {
	/*
		Idle has no internal state - it's the default/empty behavior.
	*/

	bool operator==(const ai_behavior_idle&) const = default;
};
