#include "augs/misc/hmac_sha256.h"

#if BUILD_OPENSSL
#include <openssl/evp.h>
#include <openssl/hmac.h>
#endif

namespace augs {
	std::optional<hmac_sha256_type> hmac_sha256(
		const std::byte* const key,
		const std::size_t key_size,
		const std::byte* const message,
		const std::size_t message_size
	) {
#if BUILD_OPENSSL
		auto result = hmac_sha256_type();
		unsigned result_size = 0;

		const auto* const written = HMAC(
			EVP_sha256(),
			key,
			static_cast<int>(key_size),
			reinterpret_cast<const unsigned char*>(message),
			message_size,
			result.data(),
			&result_size
		);

		if (written == nullptr || result_size != result.size()) {
			return std::nullopt;
		}

		return result;
#else
		(void)key;
		(void)key_size;
		(void)message;
		(void)message_size;

		return std::nullopt;
#endif
	}
}
