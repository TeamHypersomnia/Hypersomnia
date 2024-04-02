#pragma once

#if BUILD_OPENSSL
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "3rdparty/cpp-httplib/httplib.h"

#if PLATFORM_WEB
#include "augs/misc/httplib_emscripten.h"
using http_client_type = augs::emscripten_http;
using httplib_result = augs::emscripten_http::result;
#else
using http_client_type = httplib::Client;
using httplib_result = httplib::Result;
#endif

#undef ADD
