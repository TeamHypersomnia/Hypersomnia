#pragma once
#include <string_view>

/*
	The public key of the identity broker on hypersomnia.io - see identity_cert.h.
	A server trusts the certs it signs unless identity_broker_public_key in its config names another (e.g. a test broker).
*/

inline constexpr std::string_view default_identity_broker_public_key_v = "b1f8f0b2711cf0596c83b17f8d8904f547c2243705edc1e8c560fcc126c5cfbc";
