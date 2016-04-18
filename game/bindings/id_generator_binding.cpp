#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "misc/id_generator.h"

namespace bindings {
	luabind::scope _id_generator() {
		return
			id_generator<unsigned short>::bind("id_generator_ushort"),
			id_generator<unsigned int>::bind("id_generator_uint")
			;
	}
}