#pragma once
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include "augs/log.h"

namespace augs {
	struct self_signed_certificate_pem {
		std::string certificate;
		std::string private_key;
	};

	inline std::optional<self_signed_certificate_pem> make_self_signed_p256_certificate(
		const std::string& common_name,
		const long valid_secs
	) {
#if OPENSSL_VERSION_NUMBER >= 0x30000000
		using key_ptr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
		using x509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;
		using name_ptr = std::unique_ptr<X509_NAME, decltype(&X509_NAME_free)>;
		using bignum_ptr = std::unique_ptr<BIGNUM, decltype(&BN_free)>;
		using bio_ptr = std::unique_ptr<BIO, decltype(&BIO_free)>;

		auto failed = [](const char* const step) -> std::optional<self_signed_certificate_pem> {
			auto reason = std::array<char, 256>();
			ERR_error_string_n(ERR_get_error(), reason.data(), reason.size());
			ERR_clear_error();

			LOG("Could not generate a self-signed certificate (%x): %x", step, std::string(reason.data()));
			return std::nullopt;
		};

		const auto key = key_ptr(EVP_EC_gen("prime256v1"), EVP_PKEY_free);
		const auto x509 = x509_ptr(X509_new(), X509_free);
		const auto name = name_ptr(X509_NAME_new(), X509_NAME_free);
		const auto serial_number = bignum_ptr(BN_new(), BN_free);

		if (key == nullptr || x509 == nullptr || name == nullptr || serial_number == nullptr) {
			return failed("allocation");
		}

		const auto serial_bits = 128;
		const auto valid_since_secs_ago = 3600L;
		auto* const common_name_bytes = reinterpret_cast<const unsigned char*>(common_name.c_str());

		const bool built =
			X509_gmtime_adj(X509_getm_notBefore(x509.get()), -valid_since_secs_ago) != nullptr &&
			X509_gmtime_adj(X509_getm_notAfter(x509.get()), valid_secs) != nullptr &&
			X509_set_version(x509.get(), X509_VERSION_1) == 1 &&
			BN_rand(serial_number.get(), serial_bits, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY) == 1 &&
			BN_to_ASN1_INTEGER(serial_number.get(), X509_get_serialNumber(x509.get())) != nullptr &&
			X509_NAME_add_entry_by_NID(name.get(), NID_commonName, MBSTRING_UTF8, common_name_bytes, -1, -1, 0) == 1 &&
			X509_set_subject_name(x509.get(), name.get()) == 1 &&
			X509_set_issuer_name(x509.get(), name.get()) == 1 &&
			X509_set_pubkey(x509.get(), key.get()) == 1 &&
			X509_sign(x509.get(), key.get(), EVP_sha256()) > 0
		;

		if (!built) {
			return failed("building");
		}

		auto to_pem = [](auto write) -> std::optional<std::string> {
			const auto bio = bio_ptr(BIO_new(BIO_s_mem()), BIO_free);

			if (bio == nullptr || write(bio.get()) != 1) {
				return std::nullopt;
			}

			char* data = nullptr;
			const auto length = BIO_get_mem_data(bio.get(), &data);

			if (data == nullptr || length <= 0) {
				return std::nullopt;
			}

			return std::string(data, static_cast<std::size_t>(length));
		};

		auto certificate = to_pem([&](BIO* const bio) {
			return PEM_write_bio_X509(bio, x509.get());
		});

		auto private_key = to_pem([&](BIO* const bio) {
			return PEM_write_bio_PrivateKey(bio, key.get(), nullptr, nullptr, 0, nullptr, nullptr);
		});

		if (!certificate.has_value() || !private_key.has_value()) {
			return failed("PEM encoding");
		}

		return self_signed_certificate_pem { std::move(*certificate), std::move(*private_key) };
#else
		(void)common_name;
		(void)valid_secs;
		return std::nullopt;
#endif
	}
}
