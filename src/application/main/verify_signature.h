#pragma once
#include "augs/window_framework/pipe.h"
#include "signing_keys.h"

#if PLATFORM_WINDOWS
#define SSH_KEYGEN_BINARY (augs::path_type("scripts") / "ssh" / "ssh-keygen.exe")
#else
#define SSH_KEYGEN_BINARY "ssh-keygen"
#endif

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

	const auto allowed_signers = augs::path_type(GENERATED_FILES_DIR) / "allowed_signers";
	augs::save_as_text(allowed_signers, ::SIGNING_PUBLIC_KEY);

	const auto verification_command = typesafe_sprintf(
		"%x -Y verify -f %x -I hypersomnia -n %x -s %x",
		SSH_KEYGEN_BINARY,
		allowed_signers,
		ssh_namespace,
		signature
	);

	const std::string verification_success = std::string("Good \"") + ssh_namespace + "\" signature for hypersomnia with";

	try {
		LOG("Pipe command: %x", verification_command);
		const auto verification_result = augs::pipe::execute(verification_command, verified_file.string());
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

