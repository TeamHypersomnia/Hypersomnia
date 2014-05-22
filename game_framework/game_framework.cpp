#pragma once
#include "stdafx.h"

#include <gtest\gtest.h>
#include "augmentations.h"

namespace framework {
	void init() {
		augs::init();

		int argc = 0;
		::testing::InitGoogleTest(&argc, (wchar_t**)nullptr);

		::testing::FLAGS_gtest_catch_exceptions = false;
		::testing::FLAGS_gtest_break_on_failure = false;
		auto result = RUN_ALL_TESTS();
	}

	void deinit() {
		augs::deinit();
	}
}