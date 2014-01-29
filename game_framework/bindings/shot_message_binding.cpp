#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../messages/shot_message.h"
#include "entity_system/entity.h"

namespace bindings {
	luabind::scope _shot_message() {
		return
			luabind::class_<shot_message, message>("shot_message")
			.def(luabind::constructor<>());
	}
}