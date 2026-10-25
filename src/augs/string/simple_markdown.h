#pragma once
#include <string>
#include <vector>

/*
	A tiny markdown subset for formatted text in the GUI, e.g. the changelog.

	Lines:
		# Heading .. ###### Heading   heading, level = number of #
		- item / * item / + item      bullet, level = tabs or pairs of spaces
		plain text                    paragraph
		![alt](path)                  skipped
		| a | b |                     table row, level = indentation like a bullet's
		|---|---|                     marks the row above as the table's header
	A non-empty line right under a bullet or paragraph continues it.
	An empty line ends it. Consecutive table rows make one table.

	Inline:
		**bold** __bold__ *italic* _italic_ `code` [text](url)
	A link is its text with the url beside it (LINK); an image ![..](..) is dropped. Unbalanced markers stay literal.
*/

namespace augs {
	enum class markdown_span_style {
		NORMAL,
		BOLD,
		ITALIC,
		CODE,
		LINK
	};

	struct markdown_span {
		markdown_span_style style = markdown_span_style::NORMAL;
		std::string text;

		/*
			Only for LINK.
		*/

		std::string url = "";

		bool operator==(const markdown_span&) const = default;
	};

	enum class markdown_line_type {
		HEADING,
		BULLET,
		PARAGRAPH,
		TABLE_ROW
	};

	struct markdown_line {
		markdown_line_type type = markdown_line_type::PARAGRAPH;
		int level = 0;
		std::vector<markdown_span> spans;

		/*
			Only for TABLE_ROW.
		*/

		std::vector<std::vector<markdown_span>> cells;
		bool is_header = false;

		std::string get_plain_text() const;
	};

	std::vector<markdown_span> parse_markdown_inline(const std::string& text);
	std::vector<markdown_line> parse_markdown(const std::string& source);
}
