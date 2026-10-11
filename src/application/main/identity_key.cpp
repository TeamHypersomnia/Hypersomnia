#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <system_error>
#if PLATFORM_UNIX
#include <fcntl.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <io.h>
#endif
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/misc/secure_hash.h"
#include "augs/misc/date_time.h"
#include "application/main/identity_key.h"

namespace identity_key {
	std::string signed_message(const std::string& auth_identity) {
		return "hypersomnia-auth-v1 " + auth_identity;
	}

	std::string link_message(const std::string& code, const std::string& steam_account) {
		return "hypersomnia-link-v1 " + code + " " + steam_account;
	}

	std::string account_id(const augs::ed25519_public_key_type& public_key) {
		return augs::bytes_to_hex(public_key);
	}

	std::vector<std::byte> to_bytes(const ticket& t) {
		auto out = std::vector<std::byte>(t.public_key.size() + t.signature.size());

		std::memcpy(out.data(), t.public_key.data(), t.public_key.size());
		std::memcpy(out.data() + t.public_key.size(), t.signature.data(), t.signature.size());

		return out;
	}

	std::optional<ticket> from_bytes(const std::vector<std::byte>& bytes) {
		auto out = ticket();

		if (bytes.size() != out.public_key.size() + out.signature.size()) {
			return std::nullopt;
		}

		std::memcpy(out.public_key.data(), bytes.data(), out.public_key.size());
		std::memcpy(out.signature.data(), bytes.data() + out.public_key.size(), out.signature.size());

		return out;
	}

	std::optional<ticket> sign(const augs::ed25519_seed_type& seed, const std::string& auth_identity) {
		const auto public_key = augs::ed25519_public_key(seed);
		const auto signature = augs::ed25519_sign(seed, signed_message(auth_identity));

		if (!public_key.has_value() || !signature.has_value()) {
			return std::nullopt;
		}

		return ticket { *public_key, *signature };
	}

	bool verify(const ticket& t, const std::string& auth_identity) {
		return augs::ed25519_verify(t.public_key, signed_message(auth_identity), t.signature);
	}

	std::string to_export_string(const augs::ed25519_seed_type& seed) {
		return std::string(export_prefix) + augs::bytes_to_hex(seed);
	}

	std::optional<augs::ed25519_seed_type> from_export_string(const std::string& text) {
		/*
			Pasted keys often come with whitespace around them, e.g. copied from a chat or a text editor.
		*/

		const auto is_space = [](const char c) {
			return c == ' ' || c == '\t' || c == '\n' || c == '\r';
		};

		auto trimmed = text;

		while (!trimmed.empty() && is_space(trimmed.back())) {
			trimmed.pop_back();
		}

		const auto first = std::find_if_not(trimmed.begin(), trimmed.end(), is_space);
		trimmed.erase(trimmed.begin(), first);

		if (trimmed.rfind(export_prefix, 0) != 0) {
			return std::nullopt;
		}

		const auto hex = trimmed.substr(export_prefix.size());
		auto seed = augs::ed25519_seed_type();

		if (hex.size() != seed.size() * 2) {
			return std::nullopt;
		}

		for (std::size_t i = 0; i < seed.size(); ++i) {
			unsigned value = 0;

			for (std::size_t j = 0; j < 2; ++j) {
				const char c = hex[i * 2 + j];
				value <<= 4;

				if (c >= '0' && c <= '9') {
					value |= static_cast<unsigned>(c - '0');
				}
				else if (c >= 'a' && c <= 'f') {
					value |= static_cast<unsigned>(c - 'a' + 10);
				}
				else if (c >= 'A' && c <= 'F') {
					value |= static_cast<unsigned>(c - 'A' + 10);
				}
				else {
					return std::nullopt;
				}
			}

			seed[i] = static_cast<uint8_t>(value);
		}

		return seed;
	}

	std::optional<augs::ed25519_seed_type> load(const augs::path_type& path) {
		try {
			return from_export_string(augs::file_to_string(path));
		}
		catch (...) {
			return std::nullopt;
		}
	}

	/*
		The key is never left half-written and never overwrites a file:
		it is written to a temporary file next to it (readable by the owner only from the start),
		flushed to the disk, and only then linked under its name - which fails if that name is taken.
	*/

	static bool write_and_sync(const augs::path_type& path, const std::string& content) {
#if PLATFORM_UNIX
		const auto fd = ::open(path.string().c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);

		if (fd < 0) {
			return false;
		}

		const auto written = ::write(fd, content.data(), content.size());
		const bool synced = ::fsync(fd) == 0;
		::close(fd);

		return synced && written == static_cast<decltype(written)>(content.size());
#elif defined(_WIN32)
		std::FILE* const file = ::_wfopen(path.c_str(), L"wbx");

		if (file == nullptr) {
			return false;
		}

		const auto written = std::fwrite(content.data(), 1, content.size(), file);
		const bool synced = std::fflush(file) == 0 && ::_commit(::_fileno(file)) == 0;
		std::fclose(file);

		return synced && written == content.size();
#else
		std::FILE* const file = std::fopen(path.string().c_str(), "wbx");

		if (file == nullptr) {
			return false;
		}

		const auto written = std::fwrite(content.data(), 1, content.size(), file);
		const bool flushed = std::fflush(file) == 0;
		std::fclose(file);

		return flushed && written == content.size();
#endif
	}

	/*
		Gives the file a second name, failing if that name is taken.
		Where hard links are not supported (e.g. FAT), it is renamed instead, after checking the name is free.
	*/

	static bool link_without_replacing(const augs::path_type& from, const augs::path_type& to) {
		std::error_code ec;
		std::filesystem::create_hard_link(from, to, ec);

		if (!ec) {
			std::filesystem::remove(from, ec);
			return true;
		}

		if (std::filesystem::exists(to)) {
			return false;
		}

		std::filesystem::rename(from, to, ec);
		return !ec;
	}

	static bool write_private_file(const augs::path_type& path, const std::string& content) {
		std::error_code ec;
		std::filesystem::create_directories(path.parent_path(), ec);

		auto temporary = path;
		temporary += ".tmp-" + std::to_string(std::random_device()());

		if (!write_and_sync(temporary, content)) {
			std::filesystem::remove(temporary, ec);
			return false;
		}

		if (!link_without_replacing(temporary, path)) {
			std::filesystem::remove(temporary, ec);
			return false;
		}

		return true;
	}

	std::optional<augs::ed25519_seed_type> load_or_create(const augs::path_type& path) {
		if (augs::exists(path)) {
			const auto loaded = load(path);

			if (!loaded.has_value()) {
				LOG("%x is not a valid identity key. Leaving it as is.", path);
			}

			return loaded;
		}

		const auto seed = augs::ed25519_generate_seed();

		if (!seed.has_value()) {
			LOG("Failed to generate an identity key.");
			return std::nullopt;
		}

		if (!write_private_file(path, to_export_string(*seed) + "\n")) {
			/*
				E.g. another instance of the game created it in the meantime - that one is the key.
			*/

			if (augs::exists(path)) {
				return load(path);
			}

			LOG("Failed to save the identity key to %x.", path);
			return std::nullopt;
		}

		LOG("Created a new identity key at %x.", path);
		return seed;
	}

	/*
		Never overwrites an earlier backup, even one made within the same second.
	*/

	static std::optional<augs::path_type> back_up(const augs::path_type& path) {
		const auto stamp = augs::date_time().get_readable_for_file();

		for (int attempt = 1; attempt <= 100; ++attempt) {
			auto backup = path;
			backup += ".bak-" + stamp + (attempt == 1 ? std::string() : "-" + std::to_string(attempt));

			if (link_without_replacing(path, backup)) {
				return backup;
			}

			if (!augs::exists(path)) {
				break;
			}
		}

		LOG("Could not back up %x.", path);
		return std::nullopt;
	}

	std::optional<augs::path_type> forget(const augs::path_type& path) {
		if (!augs::exists(path)) {
			return std::nullopt;
		}

		return back_up(path);
	}

	std::optional<augs::path_type> save_replacing(const augs::path_type& path, const augs::ed25519_seed_type& seed) {
		auto backup = augs::path_type();

		if (augs::exists(path)) {
			if (const auto new_backup = back_up(path)) {
				backup = *new_backup;
			}
			else {
				return std::nullopt;
			}
		}

		if (!write_private_file(path, to_export_string(seed) + "\n")) {
			LOG("Failed to save the identity key to %x.", path);
			return std::nullopt;
		}

		return backup;
	}
}

#if BUILD_UNIT_TESTS && BUILD_OPENSSL
#include <Catch/single_include/catch2/catch.hpp>

TEST_CASE("Identity key export string round trip, with whitespace around") {
	auto seed = augs::ed25519_seed_type();

	for (std::size_t i = 0; i < seed.size(); ++i) {
		seed[i] = static_cast<uint8_t>(i * 7 + 1);
	}

	const auto exported = identity_key::to_export_string(seed);

	REQUIRE(identity_key::from_export_string(exported) == seed);
	REQUIRE(identity_key::from_export_string(exported + "\n") == seed);
	REQUIRE(identity_key::from_export_string("  \t" + exported + " \r\n") == seed);
	REQUIRE(identity_key::from_export_string("\n" + exported) == seed);

	REQUIRE_FALSE(identity_key::from_export_string("").has_value());
	REQUIRE_FALSE(identity_key::from_export_string("hypersomnia-key-v1:nothex").has_value());
	REQUIRE_FALSE(identity_key::from_export_string(exported.substr(0, exported.size() - 1)).has_value());
	REQUIRE_FALSE(identity_key::from_export_string(exported + "0").has_value());
	REQUIRE_FALSE(identity_key::from_export_string("x" + exported).has_value());
}

TEST_CASE("Identity key files are created once, replaced and forgotten only with a backup") {
	const auto dir = std::filesystem::temp_directory_path() / ("hypersomnia-identity-key-test-" + std::to_string(std::random_device()()));
	const auto path = dir / "identity.key";

	const auto created = identity_key::load_or_create(path);
	REQUIRE(created.has_value());
	REQUIRE(identity_key::load_or_create(path) == created);
	REQUIRE(identity_key::load(path) == created);

	auto other = augs::ed25519_seed_type();
	other[0] = 42;

	const auto first_backup = identity_key::save_replacing(path, other);
	REQUIRE(first_backup.has_value());
	REQUIRE(identity_key::load(*first_backup) == created);
	REQUIRE(identity_key::load(path) == other);

	/* Within the same second: a second backup does not overwrite the first */
	const auto second_backup = identity_key::forget(path);
	REQUIRE(second_backup.has_value());
	REQUIRE(*second_backup != *first_backup);
	REQUIRE(identity_key::load(*first_backup) == created);
	REQUIRE(identity_key::load(*second_backup) == other);
	REQUIRE_FALSE(std::filesystem::exists(path));
	REQUIRE_FALSE(identity_key::forget(path).has_value());

	/* No temporary files left behind */
	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		REQUIRE(entry.path().filename().string().find(".tmp-") == std::string::npos);
	}

#if PLATFORM_UNIX
	REQUIRE((std::filesystem::status(*second_backup).permissions() & std::filesystem::perms::group_read) == std::filesystem::perms::none);
	REQUIRE((std::filesystem::status(*second_backup).permissions() & std::filesystem::perms::others_read) == std::filesystem::perms::none);
#endif

	std::filesystem::remove_all(dir);
}
#endif
