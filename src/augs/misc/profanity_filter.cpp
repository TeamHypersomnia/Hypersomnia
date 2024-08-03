#include <unordered_set>
#include "augs/misc/profanity_filter.h"
#include "augs/misc/crazygames_profanities_list.h"
#include "augs/templates/container_templates.h"

struct profanities_init {
    std::unordered_set<std::string> words;
    std::vector<std::string> with_spaces;

    profanities_init() {
        for (const auto& p : profanities_list) {
			if (std::strchr(p, ' ') != nullptr) {
                with_spaces.push_back(p);
            }
			else {
                words.emplace(p);
            }
        }

        with_spaces.shrink_to_fit();
    }
} profanities;

template <class T>
void process_words(const std::string& input, T callback) {
    std::string token;

    for (size_t i = 0; i < input.length(); ++i) {
        if (std::isspace(input[i])) {
            if (!token.empty()) {
                callback(token);
                token.clear();
            }
        }
		else {
            token.push_back(input[i]);
        }
    }
    // Check the last token
    if (!token.empty()) {
        callback(token);
    }
}

std::string to_lowercase(std::string s);

std::string limit_repetitions(const std::string& str, int max_repeats = 2) {
    if (str.empty()) {
        return str;
    }

    std::string result;
    result.reserve(str.size());

    char last_char = str[0];
    int count = 1;
    result.push_back(last_char);

    for (std::size_t i = 1; i < str.size(); ++i) {
        if (str[i] == last_char && std::isalpha(str[i])) {
            if (count < max_repeats) {
                result.push_back(str[i]);
                count++;
            }
        }
		else {
            last_char = str[i];
            result.push_back(last_char);
            count = 1;
        }
    }

    return result;
}

std::string insert_space_before_caseness_change(const std::string& str) {
    std::string result;

	if (str.empty()) {
		return result;
	}

    bool last_is_lower = std::islower(str[0]);

    result.push_back(str[0]);

    for (std::size_t i = 1; i < str.size(); ++i) {
        const bool current_is_lower = std::islower(str[i]);

        if (last_is_lower && !current_is_lower) {
            result.push_back(' ');
        }

        result.push_back(str[i]);
        last_is_lower = current_is_lower;
    }

    return result;
}

namespace augs {
	bool has_profanity_base_check(const std::string& input) {
		bool found = false;

		process_words(input, [&found](const std::string& word) {
			if (found_in(profanities.words, word)) {
				found = true;
			}
		});

		if (found) {
			return true;
		}

		if (input.find(" ") == std::string::npos) {
			return false;
		}

        for (const auto& profanity : profanities.with_spaces) {
            if (input.find(profanity) != std::string::npos) {
                return true;
            }
        }

        return false;
	}

	bool has_profanity_base_check2(std::string cleaned) {
		cleaned = ::to_lowercase(cleaned);

		/* Case: words delimited by punctuations aaa.bbb */
		{
			auto pun_to_space = cleaned;

			for (auto& p : pun_to_space) {
				if (std::ispunct(p)) {
					p = ' ';
				}
			}

			if (has_profanity_base_check(pun_to_space)) {
				return true;
			}
		}

		/* Case: words with punctuations inside a.a.a.a */

		erase_if(cleaned, ::ispunct);

		if (has_profanity_base_check(cleaned)) {
			return true;
		}

		/*
			Case: words with spaces inside a a a a
			Case: words delimited by spaces are handled by definition.
		*/

		erase_if(cleaned, ::isspace);

		if (found_in(profanities.words, cleaned)) {
			return true;
		}

		return false;
	}

    bool has_profanity(const std::string& in_orig) {
		auto cleaned = limit_repetitions(in_orig);

		return
			has_profanity_base_check2(cleaned) ||
			has_profanity_base_check2(insert_space_before_caseness_change(cleaned))
		;
    }
}
