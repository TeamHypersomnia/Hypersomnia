#!/usr/bin/env python3
import re
import sys
from xml.sax.saxutils import escape

releases_placeholder = "@RELEASES@"
max_release_note_items = 9
changelog_url = "https://github.com/TeamHypersomnia/Hypersomnia/blob/master/hypersomnia/CHANGELOG.md"

version_heading = re.compile(r"^## \[(?P<version>[^\]]+)\] - (?P<date>\d{4}-\d{2}-\d{2})\s*$")
section_heading = re.compile(r"^### (?P<name>.+?)\s*$")
top_level_bullet = re.compile(r"^- (?P<text>.+)$")


def strip_markdown(text):
	text = re.sub(r"\[([^\]]+)\]\([^)]*\)", r"\1", text)
	text = re.sub(r"`([^`]*)`", r"\1", text)
	text = text.replace("**", "").replace("__", "")
	text = re.sub(r"(?<![\w*])\*(?!\s)(.+?)(?<!\s)\*(?![\w*])", r"\1", text)
	text = re.sub(r"(?<!\w)_(?!\s)(.+?)(?<!\s)_(?!\w)", r"\1", text)
	return " ".join(text.split())


def github_heading_anchor(heading_text):
	slug = re.sub(r"[^\w\- ]", "", heading_text.lower())
	return slug.replace(" ", "-")


def continues_previous_bullet(line):
	return line.strip() != "" and not line.startswith(("\t", " ", "-", "#", "|"))


def parse_changelog(changelog_text):
	releases = []
	last_bullets = None

	for line in changelog_text.splitlines():
		if match := version_heading.match(line):
			releases.append({
				"version": match["version"],
				"date": match["date"],
				"anchor": github_heading_anchor(line[3:]),
				"sections": []
			})

			last_bullets = None
			continue

		if not releases:
			continue

		sections = releases[-1]["sections"]

		if match := section_heading.match(line):
			sections.append((match["name"], []))
			last_bullets = None
			continue

		if not sections:
			continue

		if match := top_level_bullet.match(line):
			last_bullets = sections[-1][1]
			last_bullets.append(match["text"])
			continue

		if last_bullets and continues_previous_bullet(line):
			last_bullets[-1] += " " + line.strip()
			continue

		last_bullets = None

	return releases


def take_round_robin(sections, limit):
	taken = [[] for _ in sections]
	total = 0
	depth = 0

	while total < limit and any(depth < len(bullets) for _, bullets in sections):
		for index, (_, bullets) in enumerate(sections):
			if total < limit and depth < len(bullets):
				taken[index].append(bullets[depth])
				total += 1

		depth += 1

	return [(name, chosen) for (name, _), chosen in zip(sections, taken) if chosen]


def release_xml(release, indent):
	inner = indent + "  "
	lines = [
		f'{indent}<release version="{escape(release["version"])}" date="{release["date"]}">',
		f'{inner}<url type="details">{escape(changelog_url + "#" + release["anchor"])}</url>'
	]

	notes = take_round_robin(release["sections"], max_release_note_items)

	if notes:
		lines.append(f"{inner}<description>")

		for name, bullets in notes:
			lines.append(f"{inner}  <p>{escape(strip_markdown(name))}</p>")
			lines.append(f"{inner}  <ul>")

			for bullet in bullets:
				lines.append(f"{inner}    <li>{escape(strip_markdown(bullet))}</li>")

			lines.append(f"{inner}  </ul>")

		lines.append(f"{inner}</description>")

	lines.append(f"{indent}</release>")
	return "\n".join(lines)


def main():
	template_path, changelog_path, output_path = sys.argv[1:4]

	with open(template_path, encoding="utf-8") as f:
		template = f.read()

	with open(changelog_path, encoding="utf-8") as f:
		releases = parse_changelog(f.read())

	if releases_placeholder not in template:
		sys.exit(f"{template_path}: no {releases_placeholder} line to replace with the releases.")

	if not releases:
		sys.exit(f"{changelog_path}: no '## [version] - YYYY-MM-DD' headings.")

	placeholder_line = next(line for line in template.splitlines() if releases_placeholder in line)
	indent = placeholder_line[:len(placeholder_line) - len(placeholder_line.lstrip())]

	releases_block = "\n".join(release_xml(release, indent) for release in releases)

	with open(output_path, "w", encoding="utf-8") as f:
		f.write(template.replace(placeholder_line, releases_block))


main()
