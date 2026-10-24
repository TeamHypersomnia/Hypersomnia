#pragma once
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

/*
	Who has a place in which open match of a tournament's round 1 - shared by all its servers (they run in one process),
	so one account takes one place: a player with a place on Match #1 isn't given another on Match #2.
*/

class open_round_places {
	std::mutex lk;
	std::unordered_map<std::string, std::string> server_of;

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

	void take(const std::string& account, const std::string& server) {
		std::scoped_lock guard(lk);
		server_of[account] = server;
	}

	void free(const std::string& account, const std::string& server) {
		std::scoped_lock guard(lk);

		if (const auto found = server_of.find(account); found != server_of.end() && found->second == server) {
			server_of.erase(found);
		}
	}
};
