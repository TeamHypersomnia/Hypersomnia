#pragma once
#include "augs/window_framework/pipe.h"
#include "signing_keys.h"
#include "all_paths.h"

inline std::string quoted_for_ssh_keygen_command(const augs::path_type& path) {
	const auto utf8_path = augs::string_windows_friendly(path);

#if PLATFORM_UNIX
	return augs::quoted_for_shell(utf8_path);
#else
	return "\"" + utf8_path + "\"";
#endif
}

inline bool ssh_keygen_is_installed() {
#if PLATFORM_UNIX
	return augs::shell("command -v ssh-keygen > /dev/null 2>&1") == 0;
#else
	return true;
#endif
}

enum class verified_object_type {
	GAME,
	UPDATER
};

enum class ssh_verification_result {
	OK,
	NO_KEYGEN,
	FAILED_VERIFICATION
};

inline ssh_verification_result verify_ssh_signature(
	const augs::path_type& verified_file,
	const augs::path_type& signature,
	const verified_object_type type = verified_object_type::UPDATER
) {
	const auto ssh_namespace = type == verified_object_type::UPDATER ? "self_updater" : "hypersomnia";

	if (!::ssh_keygen_is_installed()) {
		LOG("ssh-keygen is not installed - the signature cannot be verified.");
		return ssh_verification_result::NO_KEYGEN;
	}

	const auto allowed_signers = CACHE_DIR / "allowed_signers";
	augs::save_as_text(allowed_signers, ::SIGNING_PUBLIC_KEY);

	const auto verification_command = typesafe_sprintf(
		"ssh-keygen -Y verify -f %x -I hypersomnia -n %x -s %x",
		::quoted_for_ssh_keygen_command(allowed_signers),
		ssh_namespace,
		::quoted_for_ssh_keygen_command(signature)
	);

	const std::string verification_success = std::string("Good \"") + ssh_namespace + "\" signature for hypersomnia with";

	try {
		LOG("Pipe command: %x", verification_command);
		const auto verification_result = augs::pipe::execute(verification_command, augs::string_windows_friendly(verified_file));
		LOG("Pipe result: %x", verification_result);

		if (::begins_with(verification_result, verification_success)) {
			return ssh_verification_result::OK;
		}
	}
	catch (const std::runtime_error& err) {
		LOG("Failed to open ssh-keygen for signature verification: %x", err.what());
		return ssh_verification_result::NO_KEYGEN;
	}

	return ssh_verification_result::FAILED_VERIFICATION;
}

