#include <cctype>
#include <sstream>
#include "augs/string/simple_markdown.h"

static bool is_word_char(const char c) {
	const auto u = static_cast<unsigned char>(c);

	/*
		Treat UTF-8 continuation bytes as letters so that _ inside non-ASCII words stays literal.
	*/

	return u >= 0x80 || std::isalnum(u) || c == '_';
}

static int indent_level(const std::string& whitespace) {
	auto level = 0;
	auto spaces = 0;

	for (const auto c : whitespace) {
		if (c == '\t') {
			++level;
			spaces = 0;
		}
		else {
			++spaces;

			if (spaces == 2) {
				++level;
				spaces = 0;
			}
		}
	}

	return level;
}

std::string augs::markdown_line::get_plain_text() const {
	auto result = std::string();

	for (const auto& span : spans) {
		result += span.text;
	}

	return result;
}

std::vector<augs::markdown_span> augs::parse_markdown_inline(const std::string& text) {
	using S = markdown_span_style;

	auto result = std::vector<markdown_span>();
	const auto n = text.size();
	const auto npos = std::string::npos;

	auto add = [&](const S style, const std::string& s) {
		if (s.empty()) {
			return;
		}

		if (!result.empty() && result.back().style == style && style != S::LINK) {
			result.back().text += s;
			return;
		}

		result.push_back({ style, s });
	};

	auto at = [&](const std::size_t i) {
		return i < n ? text[i] : '\0';
	};

	auto left_is_word = [&](const std::size_t i) {
		return i > 0 && ::is_word_char(text[i - 1]);
	};

	/*
		Closing marker must not follow a space, must not be right after the opening one,
		and - for _ and single * - must not be followed by a word character.
	*/

	auto find_closing = [&](const std::string& marker, const std::size_t from, const bool needs_word_boundary) {
		auto pos = text.find(marker, from);

		while (pos != npos) {
			const bool non_empty = pos > from;
			const bool no_space_before = text[pos - 1] != ' ';
			const bool boundary_ok = !needs_word_boundary || !::is_word_char(at(pos + marker.size()));

			if (non_empty && no_space_before && boundary_ok) {
				return pos;
			}

			pos = text.find(marker, pos + 1);
		}

		return npos;
	};

	/*
		For [label](url) starting at label_start, returns the position after the closing ) or npos.
	*/

	auto find_link_end = [&](const std::size_t label_start) {
		const auto label_end = text.find("](", label_start + 1);

		if (label_end == npos) {
			return npos;
		}

		const auto url_end = text.find(')', label_end + 2);

		if (url_end == npos) {
			return npos;
		}

		return url_end + 1;
	};

	std::size_t i = 0;

	while (i < n) {
		const auto c = text[i];

		if (c == '`') {
			const auto close = text.find('`', i + 1);

			if (close != npos && close > i + 1) {
				add(S::CODE, text.substr(i + 1, close - i - 1));
				i = close + 1;
				continue;
			}
		}
		else if ((c == '*' || c == '_') && at(i + 1) == c) {
			const bool underscore = c == '_';
			const auto marker = std::string(2, c);

			if (!(underscore && left_is_word(i)) && at(i + 2) != ' ') {
				const auto close = find_closing(marker, i + 2, underscore);

				if (close != npos) {
					add(S::BOLD, text.substr(i + 2, close - i - 2));
					i = close + 2;
					continue;
				}
			}
		}
		else if (c == '*' || c == '_') {
			const auto next = at(i + 1);

			if (!left_is_word(i) && next != ' ' && next != '\0') {
				const auto close = find_closing(std::string(1, c), i + 1, true);

				if (close != npos) {
					add(S::ITALIC, text.substr(i + 1, close - i - 1));
					i = close + 1;
					continue;
				}
			}
		}
		else if (c == '!' && at(i + 1) == '[') {
			if (const auto end = find_link_end(i + 1); end != npos) {
				i = end;
				continue;
			}
		}
		else if (c == '[') {
			if (const auto end = find_link_end(i); end != npos) {
				const auto label_end = text.find("](", i + 1);
				const auto label = text.substr(i + 1, label_end - i - 1);

				if (!label.empty()) {
					result.push_back({ S::LINK, label, text.substr(label_end + 2, end - 1 - (label_end + 2)) });
				}

				i = end;
				continue;
			}
		}

		add(S::NORMAL, std::string(1, c));
		++i;
	}

	return result;
}

std::vector<augs::markdown_line> augs::parse_markdown(const std::string& source) {
	using L = markdown_line_type;

	struct raw_line {
		L type;
		int level;
		std::string text;
		std::vector<std::string> cells = {};
		bool is_header = false;
	};

	/*
		"| a | b |" -> "a", "b". The outer pipes are optional.
	*/

	auto split_table_row = [](std::string row) {
		if (!row.empty() && row.front() == '|') {
			row.erase(0, 1);
		}

		if (!row.empty() && row.back() == '|') {
			row.pop_back();
		}

		auto cells = std::vector<std::string>();
		auto cell_stream = std::istringstream(row);

		for (std::string cell; std::getline(cell_stream, cell, '|'); ) {
			const auto first = cell.find_first_not_of(" \t");
			const auto last = cell.find_last_not_of(" \t");

			cells.push_back(first == std::string::npos ? std::string() : cell.substr(first, last - first + 1));
		}

		return cells;
	};

	auto is_header_separator = [](const std::vector<std::string>& cells) {
		for (const auto& cell : cells) {
			if (cell.empty() || cell.find_first_not_of(":-") != std::string::npos || cell.find('-') == std::string::npos) {
				return false;
			}
		}

		return !cells.empty();
	};

	auto raw_lines = std::vector<raw_line>();
	bool continuable = false;

	auto s = std::istringstream(source);

	for (std::string line; std::getline(s, line); ) {
		if (const auto last = line.find_last_not_of(" \t\r"); last != std::string::npos) {
			line.erase(last + 1);
		}
		else {
			continuable = false;
			continue;
		}

		const auto ws_end = line.find_first_not_of(" \t");
		const auto trimmed = line.substr(ws_end);

		const bool is_image =
			trimmed.rfind("![", 0) == 0
			&& trimmed.back() == ')'
			&& trimmed.find("](") != std::string::npos
		;

		if (is_image) {
			continue;
		}

		if (trimmed.front() == '|') {
			const auto cells = split_table_row(trimmed);
			const auto level = ::indent_level(line.substr(0, ws_end));
			const auto num_lines = raw_lines.size();

			auto is_row_at_level = [&](const std::size_t i) {
				return raw_lines[i].type == L::TABLE_ROW && raw_lines[i].level == level;
			};

			/*
				The separator marks the header only right under the first row of a table -
				anywhere else a row of dashes is just a row.
			*/

			const bool under_first_row =
				num_lines >= 1
				&& is_row_at_level(num_lines - 1)
				&& !(num_lines >= 2 && is_row_at_level(num_lines - 2))
			;

			if (under_first_row && is_header_separator(cells)) {
				raw_lines.back().is_header = true;
			}
			else {
				raw_lines.push_back({ L::TABLE_ROW, level, "", cells });
			}

			continuable = false;
			continue;
		}

		const auto num_hashes = line.find_first_not_of('#');

		if (num_hashes >= 1 && num_hashes <= 6 && line[num_hashes] == ' ') {
			raw_lines.push_back({ L::HEADING, static_cast<int>(num_hashes), line.substr(num_hashes + 1) });
			continuable = false;
			continue;
		}

		const bool is_bullet =
			trimmed.size() > 1
			&& (trimmed[0] == '-' || trimmed[0] == '*' || trimmed[0] == '+')
			&& trimmed[1] == ' '
		;

		if (is_bullet) {
			const auto text = trimmed.substr(trimmed.find_first_not_of(' ', 1));

			raw_lines.push_back({ L::BULLET, ::indent_level(line.substr(0, ws_end)), text });
			continuable = true;
			continue;
		}

		if (continuable && !raw_lines.empty()) {
			raw_lines.back().text += " " + trimmed;
			continue;
		}

		raw_lines.push_back({ L::PARAGRAPH, 0, trimmed });
		continuable = true;
	}

	auto result = std::vector<markdown_line>();
	result.reserve(raw_lines.size());

	for (const auto& raw : raw_lines) {
		auto& added = result.emplace_back();

		added.type = raw.type;
		added.level = raw.level;
		added.spans = parse_markdown_inline(raw.text);
		added.is_header = raw.is_header;

		for (const auto& cell : raw.cells) {
			added.cells.push_back(parse_markdown_inline(cell));
		}
	}

	return result;
}

#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>

TEST_CASE("SimpleMarkdownInline", "[markdown]") {
	using namespace augs;
	using S = markdown_span_style;
	using V = std::vector<markdown_span>;

	REQUIRE(parse_markdown_inline("") == V {});
	REQUIRE(parse_markdown_inline("plain text") == V { { S::NORMAL, "plain text" } });

	REQUIRE(parse_markdown_inline("**Balance:** cheaper") == V {
		{ S::BOLD, "Balance:" },
		{ S::NORMAL, " cheaper" }
	});

	REQUIRE(parse_markdown_inline("a __b__ c") == V {
		{ S::NORMAL, "a " },
		{ S::BOLD, "b" },
		{ S::NORMAL, " c" }
	});

	REQUIRE(parse_markdown_inline("their *Clan* and _Clan_.") == V {
		{ S::NORMAL, "their " },
		{ S::ITALIC, "Clan" },
		{ S::NORMAL, " and " },
		{ S::ITALIC, "Clan" },
		{ S::NORMAL, "." }
	});

	REQUIRE(parse_markdown_inline("see `is_idle()` now") == V {
		{ S::NORMAL, "see " },
		{ S::CODE, "is_idle()" },
		{ S::NORMAL, " now" }
	});

	REQUIRE(parse_markdown_inline("go to [the site](https://hypersomnia.io).") == V {
		{ S::NORMAL, "go to " },
		{ S::LINK, "the site", "https://hypersomnia.io" },
		{ S::NORMAL, "." }
	});

	REQUIRE(parse_markdown_inline("a ![img](x.png) b") == V { { S::NORMAL, "a  b" } });

	/*
		Unbalanced or in-word markers stay literal.
	*/

	REQUIRE(parse_markdown_inline("snake_case_name") == V { { S::NORMAL, "snake_case_name" } });
	REQUIRE(parse_markdown_inline("2 * 3 * 4") == V { { S::NORMAL, "2 * 3 * 4" } });
	REQUIRE(parse_markdown_inline("**open") == V { { S::NORMAL, "**open" } });
	REQUIRE(parse_markdown_inline("tick ` alone") == V { { S::NORMAL, "tick ` alone" } });
	REQUIRE(parse_markdown_inline("[not a link]") == V { { S::NORMAL, "[not a link]" } });
}

TEST_CASE("SimpleMarkdownLines", "[markdown]") {
	using namespace augs;
	using L = markdown_line_type;

	const auto lines = parse_markdown(
		"### Added\r\n"
		"\n"
		"- **Bots.**\n"
		"\t- Nested\n"
		"    - Deeper\n"
		"  continued\n"
		"\n"
		"![image](docs/a.png)\n"
		"Intro line\n"
		"second line\n"
		"\n"
		"* star bullet\n"
		"**bold continuation**\n"
		"\n"
		"#nospace\n"
	);

	REQUIRE(lines.size() == 7);

	REQUIRE(lines[0].type == L::HEADING);
	REQUIRE(lines[0].level == 3);
	REQUIRE(lines[0].get_plain_text() == "Added");

	REQUIRE(lines[1].type == L::BULLET);
	REQUIRE(lines[1].level == 0);
	REQUIRE(lines[1].spans.size() == 1);
	REQUIRE(lines[1].spans[0].style == markdown_span_style::BOLD);

	REQUIRE(lines[2].type == L::BULLET);
	REQUIRE(lines[2].level == 1);

	REQUIRE(lines[3].type == L::BULLET);
	REQUIRE(lines[3].level == 2);
	REQUIRE(lines[3].get_plain_text() == "Deeper continued");

	REQUIRE(lines[4].type == L::PARAGRAPH);
	REQUIRE(lines[4].get_plain_text() == "Intro line second line");

	/*
		A line right under a bullet continues it, even if it starts with a marker.
	*/

	REQUIRE(lines[5].type == L::BULLET);
	REQUIRE(lines[5].get_plain_text() == "star bullet bold continuation");
	REQUIRE(lines[5].spans.size() == 2);

	REQUIRE(lines[6].type == L::PARAGRAPH);
	REQUIRE(lines[6].get_plain_text() == "#nospace");
}

TEST_CASE("SimpleMarkdownTables", "[markdown]") {
	using namespace augs;
	using L = markdown_line_type;

	const auto lines = parse_markdown(
		"- **Balance:** SMG sprint speed.\n"
		"\t| Weapon | Before | After |\n"
		"\t|---|:--:|---|\n"
		"\t| **Pro90** | 913 | 917 |\n"
		"\t|Zamiec|885|910|\n"
		"\n"
		"| x | - |\n"
	);

	REQUIRE(lines.size() == 5);

	REQUIRE(lines[1].type == L::TABLE_ROW);
	REQUIRE(lines[1].level == 1);
	REQUIRE(lines[1].is_header);
	REQUIRE(lines[1].cells.size() == 3);

	REQUIRE(lines[2].type == L::TABLE_ROW);
	REQUIRE(!lines[2].is_header);
	REQUIRE(lines[2].cells[0].size() == 1);
	REQUIRE(lines[2].cells[0][0].style == markdown_span_style::BOLD);
	REQUIRE(lines[2].cells[2][0].text == "917");

	REQUIRE(lines[3].cells[0][0].text == "Zamiec");

	/*
		A lone dash is a cell, not a separator.
	*/

	REQUIRE(lines[4].type == L::TABLE_ROW);
	REQUIRE(lines[4].level == 0);
	REQUIRE(lines[4].cells[1][0].text == "-");
}

TEST_CASE("SimpleMarkdownTableDashRows", "[markdown]") {
	using namespace augs;

	/*
		Only the row under the first one is a separator - a row of dashes further down is data.
	*/

	const auto lines = parse_markdown(
		"| a | b |\n"
		"|---|---|\n"
		"| 1 | 2 |\n"
		"| - | - |\n"
	);

	REQUIRE(lines.size() == 3);
	REQUIRE(lines[0].is_header);
	REQUIRE(!lines[1].is_header);
	REQUIRE(!lines[2].is_header);
	REQUIRE(lines[2].cells[0][0].text == "-");
}
#endif
