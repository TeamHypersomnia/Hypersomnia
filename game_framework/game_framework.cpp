#pragma once
#include "stdafx.h"
#include "game_framework.h"

#include <gtest\gtest.h>
#include "augmentations.h"

#include "utilities/script.h"
#include "world_instance.h"

namespace framework {
	void init(unsigned which) {
		augs::init(which);
	}

	void run_tests() {
		int argc = 0;
		::testing::InitGoogleTest(&argc, (wchar_t**)nullptr);

		::testing::FLAGS_gtest_catch_exceptions = false;
		::testing::FLAGS_gtest_break_on_failure = false;
		auto result = RUN_ALL_TESTS();
	}

	void deinit() {
		augs::deinit();
	}

	void set_current_window(augs::window::glwindow& gl) {
		world_instance::global_window = &gl;
		gl.set_show(gl.SHOW);
		gl.current();
	}

}