#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <string_view>
#include <system_error>
#if PLATFORM_UNIX
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#if PLATFORM_LINUX && !PLATFORM_WEB
#include <linux/fs.h>
#include <sys/syscall.h>
#endif
#elif defined(_WIN32)
#include <io.h>
#endif
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/misc/secure_hash.h"
#include "augs/misc/date_time.h"
#include "augs/persistent_filesystem.h"
#include "augs/string/hex.h"
#include "application/main/identity_key.h"

namespace identity_key {
	std::string link_message(const std::string& code, const std::string& steam_account) {
		return "hypersomnia-link-v1 " + code + " " + steam_account;
	}

	std::string unlink_message(const augs::ed25519_public_key_type& public_key, const int64_t unix_time) {
		return "hypersomnia-unlink-v1 " + augs::bytes_to_hex(public_key) + " " + std::to_string(unix_time);
	}

	std::string account_id(const augs::ed25519_public_key_type& public_key) {
		return augs::bytes_to_hex(public_key);
	}

	std::string short_form(const augs::ed25519_public_key_type& public_key) {
		return short_form_of_account("key_" + account_id(public_key));
	}

	std::string short_form_of_account(const std::string& account) {
		if (account.rfind("key_", 0) == 0 && account.size() >= 4 + 8) {
			return "0x" + account.substr(4, 8) + "...";
		}

		return account;
	}

	std::string to_export_string(const augs::ed25519_seed_type& seed) {
		return std::string(export_prefix) + augs::bytes_to_hex(seed);
	}

	std::optional<augs::ed25519_seed_type> from_export_string(const std::string& text) {
		/*
			Key files often come with whitespace around the key, e.g. saved again by a text editor.
		*/

		const auto is_space = [](const char c) {
			return c == ' ' || c == '\t' || c == '\n' || c == '\r';
		};

		/*
			A key file saved again by a text editor can come back with a byte order mark, or even as UTF-16.
			The key itself is plain ASCII, so the UTF-16 is read by dropping the zero bytes.
		*/

		auto trimmed = [&]() {
			const auto starts_with_bytes = [&](const std::string_view prefix) {
				return text.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), text.begin());
			};

			if (starts_with_bytes("\xEF\xBB\xBF")) {
				return text.substr(3);
			}

			if (starts_with_bytes("\xFF\xFE") || starts_with_bytes("\xFE\xFF")) {
				auto ascii = std::string();

				for (std::size_t i = 2; i < text.size(); ++i) {
					if (text[i] != '\0') {
						ascii += text[i];
					}
				}

				return ascii;
			}

			return text;
		}();

		while (!trimmed.empty() && is_space(trimmed.back())) {
			trimmed.pop_back();
		}

		const auto first = std::find_if_not(trimmed.begin(), trimmed.end(), is_space);
		trimmed.erase(trimmed.begin(), first);

		if (trimmed.rfind(export_prefix, 0) != 0) {
			return std::nullopt;
		}

		return augs::hex_to_array<32>(std::string_view(trimmed).substr(export_prefix.size()));
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

	static void sync_directory_of(const augs::path_type& path) {
#if PLATFORM_UNIX
		const auto fd = ::open(path.parent_path().string().c_str(), O_RDONLY | O_DIRECTORY);

		if (fd >= 0) {
			::fsync(fd);
			::close(fd);
		}
#else
		(void)path;
#endif
	}

	/*
		Renames, failing if the new name is taken - atomically where the system can, else after checking the name is free.
	*/

	static bool rename_without_replacing(const augs::path_type& from, const augs::path_type& to) {
#if PLATFORM_LINUX && !PLATFORM_WEB
		if (::syscall(SYS_renameat2, AT_FDCWD, from.string().c_str(), AT_FDCWD, to.string().c_str(), RENAME_NOREPLACE) == 0) {
			return true;
		}

		if (errno != EINVAL && errno != ENOSYS) {
			return false;
		}
#elif PLATFORM_MACOS
		if (::renamex_np(from.string().c_str(), to.string().c_str(), RENAME_EXCL) == 0) {
			return true;
		}

		if (errno != ENOTSUP) {
			return false;
		}
#endif

		std::error_code ec;

		if (std::filesystem::exists(to, ec)) {
			return false;
		}

		std::filesystem::rename(from, to, ec);
		return !ec;
	}

	/*
		Moves the file to a new name, failing if that name is taken.
		A hard link first, so the file is never missing under both names; renamed where links are not supported (e.g. FAT).
		Fails, leaving everything as it was, if the old name can not be removed - e.g. the file is held open on Windows.
	*/

	static bool link_without_replacing(const augs::path_type& from, const augs::path_type& to) {
		std::error_code ec;
		std::filesystem::create_hard_link(from, to, ec);

		if (!ec) {
			std::filesystem::remove(from, ec);

			if (ec) {
				LOG("Could not remove %x after linking it as %x: %x", from, to, ec.message());

				std::error_code undo_ec;
				std::filesystem::remove(to, undo_ec);
				return false;
			}

			sync_directory_of(to);
			return true;
		}

		if (std::filesystem::exists(to, ec)) {
			return false;
		}

		if (!rename_without_replacing(from, to)) {
			return false;
		}

		sync_directory_of(to);
		return true;
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

		/*
			The web build keeps the user folder in the browser's storage - written there only on a sync.
		*/

		::persistent_filesystem_sync();
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
				::persistent_filesystem_sync();
				return backup;
			}

			if (!augs::exists(path)) {
				break;
			}
		}

		LOG("Could not back up %x.", path);
		return std::nullopt;
	}

	void remove_backups(const augs::path_type& path) {
		const auto prefix = path.filename().string() + ".bak-";
		auto removed = false;
		auto ec = std::error_code();

		for (const auto& entry : std::filesystem::directory_iterator(path.parent_path(), ec)) {
			if (entry.path().filename().string().rfind(prefix, 0) == 0) {
				removed = std::filesystem::remove(entry.path(), ec) || removed;
			}
		}

		if (removed) {
			LOG("Removed the backups of %x.", path);
			::persistent_filesystem_sync();
		}
	}

	/*
		In the browser nobody can reach a backup file - a copy of the old key in the storage only waits to leak.
		Made all the same, so a failed write can still put the old key back.
	*/

	static void remove_backups_in_browser(const augs::path_type& path) {
#if PLATFORM_WEB
		remove_backups(path);
#else
		(void)path;
#endif
	}

	std::optional<augs::path_type> forget(const augs::path_type& path) {
		if (!augs::exists(path)) {
			return std::nullopt;
		}

		/*
			The browser's storage gets only the end state in one sync - never "a backup and no key".
		*/

		auto one_sync = hold_persistent_filesystem_raii();

		const auto backup = back_up(path);

		if (backup.has_value()) {
			remove_backups_in_browser(path);
		}

		return backup;
	}

	std::optional<augs::path_type> save_replacing(const augs::path_type& path, const augs::ed25519_seed_type& seed) {
		auto one_sync = hold_persistent_filesystem_raii();
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

			/*
				Put the previous key back - otherwise the next secure connection would quietly create a new account.
			*/

			if (!backup.empty() && !link_without_replacing(backup, path)) {
				LOG("Could not restore %x from %x.", path, backup);
			}

			return std::nullopt;
		}

		remove_backups_in_browser(path);
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

	/* Saved again by a text editor: a UTF-8 byte order mark, or UTF-16 */
	REQUIRE(identity_key::from_export_string("\xEF\xBB\xBF" + exported + "\r\n") == seed);

	auto utf16 = std::string("\xFF\xFE");

	for (const auto c : exported + "\r\n") {
		utf16 += c;
		utf16 += '\0';
	}

	REQUIRE(identity_key::from_export_string(utf16) == seed);

	REQUIRE_FALSE(identity_key::from_export_string("").has_value());
	REQUIRE_FALSE(identity_key::from_export_string("hypersomnia-key-v1:nothex").has_value());
	REQUIRE_FALSE(identity_key::from_export_string(exported.substr(0, exported.size() - 1)).has_value());
	REQUIRE_FALSE(identity_key::from_export_string(exported + "0").has_value());
	REQUIRE_FALSE(identity_key::from_export_string("x" + exported).has_value());
}

TEST_CASE("Identity key files are created once, replaced and forgotten only with a backup") {
	const auto dir = std::filesystem::temp_directory_path() / ("hypersomnia-identity-key-test-" + std::to_string(std::random_device()()));
	const auto path = dir / "account.key";

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
