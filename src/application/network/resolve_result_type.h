#pragma once

enum class resolve_result_type {
	OK,
	INVALID_ADDRESS,
	COULDNT_RESOLVE_HOST,
	IPV6_NOT_SUPPORTED
};
