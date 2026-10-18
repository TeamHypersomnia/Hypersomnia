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
	A non-empty line right under a bullet or paragraph continues it.
	An empty line ends it.

	Inline:
		**bold** __bold__ *italic* _italic_ `code` [text](url)
	Links keep only the text. Unbalanced markers stay literal.
*/

namespace augs {
	enum class markdown_span_style {
		NORMAL,
		BOLD,
		ITALIC,
		CODE
	};

	struct markdown_span {
		markdown_span_style style = markdown_span_style::NORMAL;
		std::string text;

		bool operator==(const markdown_span&) const = default;
	};

	enum class markdown_line_type {
		HEADING,
		BULLET,
		PARAGRAPH
	};

	struct markdown_line {
		markdown_line_type type = markdown_line_type::PARAGRAPH;
		int level = 0;
		std::vector<markdown_span> spans;

		std::string get_plain_text() const;
	};

	std::vector<markdown_span> parse_markdown_inline(const std::string& text);
	std::vector<markdown_line> parse_markdown(const std::string& source);
}
