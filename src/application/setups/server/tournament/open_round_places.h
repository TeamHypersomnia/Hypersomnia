#pragma once
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

/*
	Who has a place in which open match of a tournament's round 1 - shared by all its servers (they run in one process),
	so one account takes one place: a player with a place on Match #1 isn't given another on Match #2.

	And the nicknames they took it with: a nickname is one player's for the whole round (it only grows) - so two
	who forgot to set theirs aren't both "Player" on the page.
*/

class open_round_places {
	std::mutex lk;
	std::unordered_map<std::string, std::string> server_of;
	std::unordered_map<std::string, std::string> account_of_nickname;

public:
	/*
		The server this account has its place on, if another than this one.
	*/

	std::optional<std::string> elsewhere(const std::string& account, const std::string& this_server) {
		std::scoped_lock guard(lk);

		if (const auto found = server_of.find(account); found != server_of.end() && found->second != this_server) {
			return found->second;
		}

		return std::nullopt;
	}

	bool nickname_taken(const std::string& nickname, const std::string& account) {
		std::scoped_lock guard(lk);

		const auto found = account_of_nickname.find(nickname);
		return found != account_of_nickname.end() && found->second != account;
	}

	/*
		Without a nickname: a place of a decided match, after a restart - its players took theirs already.
	*/

	void take(const std::string& account, const std::string& server, const std::string& nickname = "") {
		std::scoped_lock guard(lk);
		server_of[account] = server;

		if (!nickname.empty()) {
			account_of_nickname.try_emplace(nickname, account);
		}
	}

	void free(const std::string& account, const std::string& server) {
		std::scoped_lock guard(lk);

		if (const auto found = server_of.find(account); found != server_of.end() && found->second == server) {
			server_of.erase(found);
		}
	}
};
