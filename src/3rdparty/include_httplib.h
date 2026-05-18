#pragma once

#if BUILD_OPENSSL
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#if PLATFORM_WEB
/*
	cpp-httplib v0.32.0+ emits a #warning on 32-bit platforms (Emscripten is 32-bit).
	On web we only use the httplib types at compile time; actual HTTP goes through
	augs::emscripten_http, so the warning is harmless and must not break -Werror.
*/
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-W#warnings"
#endif

#include "3rdparty/cpp-httplib/httplib.h"

#if PLATFORM_WEB
#pragma clang diagnostic pop
#endif

#if PLATFORM_WEB
#include "augs/misc/httplib_emscripten.h"
using http_client_type = augs::emscripten_http;
using httplib_result = augs::emscripten_http::result;
#else
using http_client_type = httplib::Client;
using httplib_result = httplib::Result;
#endif

#undef ADD
