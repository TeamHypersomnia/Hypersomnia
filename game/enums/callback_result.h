#pragma once

enum class callback_result {
	CONTINUE,
	ABORT
};

enum class recursive_callback_result {
	CONTINUE_AND_RECURSE,
	CONTINUE_DONT_RECURSE,
	ABORT
};