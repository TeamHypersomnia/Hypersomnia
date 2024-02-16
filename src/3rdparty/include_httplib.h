#pragma once

#if BUILD_OPENSSL
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "3rdparty/cpp-httplib/httplib.h"

using http_client_type = httplib::Client;

#undef ADD
