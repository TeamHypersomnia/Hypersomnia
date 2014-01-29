#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../messages/message.h"
#include "entity_system/entity.h"

namespace bindings {
	luabind::scope _message() {
		return
			luabind::class_<message>("message")
			.def(luabind::constructor<>())
			.def_readwrite("subject", &message::subject);
	}
}