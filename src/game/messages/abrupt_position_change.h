#pragma once
#include "game/messages/message.h"
#include "game/components/transform_component.h"

namespace messages {
	/*
		Posted when the subject's logic transform changes discontinuously,
		e.g. when it teleports through a portal.

		Read by the audiovisual state so that effects chasing the subject
		(e.g. particle streams) do not interpolate across the jump,
		and can instead properly finish spawning at the last known position.
	*/

	struct abrupt_position_change : message {
		transformr before_change;
	};
}
