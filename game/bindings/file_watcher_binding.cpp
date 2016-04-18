#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "misc/file_watcher.h"

namespace bindings {
	luabind::scope _file_watcher() {
		return
			luabind::class_<misc::file_watcher>("file_watcher")
			.def(luabind::constructor<>())
			.def("add_directory", &file_watcher::add_directory)
			.def("get_modified_files", &file_watcher::get_modified_files);
	}
}