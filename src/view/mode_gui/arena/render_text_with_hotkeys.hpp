#pragma once
#include <string>
#include <regex>
#include <unordered_map>

#include "augs/string/string_to_enum.h"

template <class T, class H>
void render_text_with_hotkeys_line(const std::string& input, T&& text, H&& hotkey) {
    std::regex r(R"(\{(\w+)\})");
    std::smatch match;

	const auto& inventory_map = augs::get_string_to_enum_map<inventory_gui_intent_type>();
	const auto& game_map = augs::get_string_to_enum_map<game_intent_type >();
	const auto& general_map = augs::get_string_to_enum_map<general_gui_intent_type>();

    std::string::const_iterator searchStart(input.cbegin());

    while (std::regex_search(searchStart, input.cend(), match, r)) {
        std::string matched_key = match[1];
        text(input.substr(searchStart - input.cbegin(), match.position()));

        if (inventory_map.count(matched_key)) {
			hotkey(inventory_map.at(matched_key));
        } else if (game_map.count(matched_key)) {
			hotkey(game_map.at(matched_key));
        } else if (general_map.count(matched_key)) {
			hotkey(general_map.at(matched_key));
        }

        searchStart += match.position() + match.length();
    }

    text(input.substr(searchStart - input.cbegin()));
}

template <class T, class L, class H>
void render_text_with_hotkeys(const std::string& input, T&& text, L&& break_line, H&& hotkey) {
    std::istringstream iss(input);
    std::string line;
    bool firstLine = true;

    while (std::getline(iss, line)) {
        if (!firstLine) {
            break_line();
        }
        
        render_text_with_hotkeys_line(line, std::forward<T>(text), std::forward<H>(hotkey));
        firstLine = false;
    }
}
