---
name: changelog
description: Write or update CHANGELOG.md entries for a new Hypersomnia version so they render correctly in the in-game changelog (ImGui view parsed at runtime by augs/string/simple_markdown). Use whenever the user says "napisz changelog", "dodaj do changeloga", "changelog do X", "zrób changelog", "write the changelog", "add a changelog entry", or prepares a release/tag.
---

# Writing the changelog

The real file is `hypersomnia/CHANGELOG.md`. The repository root `CHANGELOG.md` is a symlink to it.
It ships with the game and is parsed at runtime (`application/gui/changelog_entry.cpp` + `augs/string/simple_markdown.cpp`).
It opens as a window once per release until the player acknowledges it (OK, X or Escape), and from the Changelog button next to the leaderboards' "Welcome home,".
Versions older than `CHANGELOG_SINCE_VERSION` (`changelog_entry.h`) are not shown in-game.

The parser understands only a small markdown subset. Anything else shows up as literal text.

## Gathering content

1. Find the previous version: the newest `## [X.Y.Z]` in `CHANGELOG.md`, and its tag (`git tag --sort=-creatordate | head`).
2. Read `git log --oneline <prev_tag>..HEAD`. Commit messages are long and list every detail - condense them.
3. Ask the user for the new version number and date only if not given. Date is the release date, `YYYY-MM-DD`.

## Format

```markdown
## [3.1.0] - 2026-10-04

### Added

- **Changelog.** Shows what's new after every update.
- Grenade trajectories.
	- Red where blocked.

### Changed

- **Balance:** Galilea: 34 -> 24 bullets.

### Fixed

- Crash when leaving the editor.
```

Rules:

1. New version goes at the top, right under the intro (newest first). The in-game list is in file order.
2. Version line exactly `## [X.Y.Z] - YYYY-MM-DD`. The version must be parsable `X.Y.Z` - it decides `(NEW!)` markers and whether the changelog pops up.
   `## [Unreleased]` is skipped in-game.
3. Section headings with `###`. Use `Added`, `Changed`, `Fixed`, `Removed` - they get their own colors (green, amber, blue, red). Other names render white.
4. Bullets with `- `. Nest with one tab per level. A non-empty line right under a bullet continues it - leave an empty line before a new paragraph.
   A top-level bullet with nested ones is drawn collapsed with a + and expands on click.
   So: a bold headline, and its explanation as nested points (`- **Game speeds.**` + `\t- Servers run at 0.75x by default.`).
   Short items stay on one line without nesting (`- **Changelog.** You're reading it.`).
5. Inline: `**bold**`, `*italic*` / `_italic_`, `` `code` ``. Bold is the main emphasis - use it for headline features and `**Balance:**`-style prefixes.
   Links `[text](url)` show only the text. Images `![..](..)` on their own line are dropped.
6. Tables: `| a | b |` rows, `|---|---|` under the header row (drawn dimmer). Indent the rows with a tab under a bullet
   so they expand with it, like its nested points. Put an empty line before the header row - without it GitHub
   glues a table that follows a nested point into that point's text (the game skips empty lines, so both render).
   Cells don't wrap - keep them short. Bold marks the new value. Two tables at one indent need a line of text
   between them - an empty line alone glues them into one.
   Balance changes go as a `- **Balance:** <what>.` headline with a table under it, e.g. `| Weapon | Before | After |`, the old value first.
7. No HTML, no numbered lists (they render as plain paragraphs).
8. Latin characters only - no arrows (→), emoji or other symbols: the game's fonts have no glyphs for them and show `?`.
   Write `Controls > Inventory`, not `Controls → Inventory`.
9. Plain paragraphs are fine for a short intro under a heading, e.g. `Most bugs were introduced by bots in 1.3.0:`.

## Style

- Write for players, not developers. "Bots now plant the bomb", not "`arena_mode_ai_state` gained a plant goal".
- Headline features first, bold, one short sentence each. Details go to nested bullets.
- Keep internal refactors, audits and code renames out unless they change behavior players notice.
- Keep lines short. The in-game view is narrow (about 40% of the screen minus the version column).

## Verify

1. Run a development build and open the changelog (the Changelog button next to the leaderboards' "Welcome home,"). It reloads `hypersomnia/CHANGELOG.md` within a second of every save - production builds read it once.
2. Stray `*`, `_` or `` ` `` in the rendered text mean unbalanced markers.
