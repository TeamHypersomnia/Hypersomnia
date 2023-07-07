#pragma once

#if BUILD_OPENSSL
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "3rdparty/cpp-httplib/httplib.h"

#if BUILD_OPENSSL
using http_client_type = httplib::SSLClient;
#else
using http_client_type = httplib::Client;
#endif

#undef ADD
