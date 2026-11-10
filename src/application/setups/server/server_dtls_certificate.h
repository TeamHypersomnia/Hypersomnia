#pragma once
#include <exception>
#include <optional>
#include "augs/log.h"
#include "augs/network/self_signed_certificate.h"
#include "rtc/rtc.hpp"

constexpr long server_dtls_certificate_valid_secs_v = 3600L * 24 * 365;

inline const std::optional<augs::self_signed_certificate_pem>& shared_server_dtls_certificate() {
	static const auto certificate = augs::make_self_signed_p256_certificate("hypersomnia", server_dtls_certificate_valid_secs_v);
	return certificate;
}

inline void use_shared_server_dtls_certificate(rtc::Configuration& config) {
	const auto& certificate = ::shared_server_dtls_certificate();

	if (!certificate.has_value()) {
		LOG("No shared DTLS certificate - every peer connection generates its own.");
		return;
	}

	config.certificatePemFile = certificate->certificate;
	config.keyPemFile = certificate->private_key;

	try {
		const auto probe = rtc::PeerConnection(config);
	}
	catch (const std::exception& err) {
		LOG("The shared DTLS certificate was refused (%x) - every peer connection generates its own.", err.what());

		config.certificatePemFile.reset();
		config.keyPemFile.reset();
	}
}
