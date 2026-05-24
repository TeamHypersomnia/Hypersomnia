#pragma once
#include <string>
#include "augs/string/string_templates.h"

/*
	When authenticate_with_nicknames is set, the server derives the authenticated
	account id from the client's chosen nickname by prefixing it with "nick_".
	Anything matching against authenticated_id (assigned teams, tournament rosters)
	must use the same prefixed form.
*/

constexpr const char* nickname_account_id_prefix_v = "nick_";

inline std::string nickname_to_account_id(const std::string& nickname) {
	return std::string(nickname_account_id_prefix_v) + nickname;
}

inline std::string account_id_to_nickname(const std::string& account_id) {
	return cut_preffix(std::string(account_id), nickname_account_id_prefix_v);
}
