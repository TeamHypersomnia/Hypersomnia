---
name: audit-green-red
description: Audit a Hypersomnia feature with an HTML report where every finding is green or red by how much complexity its fix adds, optionally fixing the green ones right away. Use only when the prompt says "z raportem złożonościowym" (or "z raportem złożoności", "zielony/czerwony raport") and the audit is the session's first task, in a fresh session. Inside a longer working session use `audit-rounds` instead.
---

# Audit with a complexity report

The audit itself is `audit-gameplay`: read `.claude/skills/audit-gameplay/SKILL.md` and follow its Scoping and
Checklist (one `auditor` agent per system, in parallel). This skill changes only two things: how each finding is
colored, and whether green findings get fixed.

## Green or red

Color decides the fix, not the bug. A crash with a one-line fix is green; a nit whose fix adds a flag is red.

- **Green** - the fix keeps or removes complexity: a one-liner, a guard, a wrong constant, deleting dead or duplicated
  code, moving code to where it belongs, a stale comment. The intended behavior is obvious.
- **Red** - the fix adds complexity (a new field, map, flag, setting, second code path, new file, new sync), or it
  is a product/UX choice, or intended behavior is unclear (severity `decision`), or it changes gameplay balance or
  what players see.

When a red finding has a cheaper variant that cuts something instead, write that variant into the fix field.

## Fixing

- Default: report only, nothing edited. Every item gets state `todo`.
- When the prompt also says **"oczywiste fixy rób sam"** (or "zielone popraw sam"): fix every green finding,
  compile-check each edited file with `build_file`, then one full `build` at the end. Mark it `fixed`.
  Red ones stay `todo` for the user. Do not commit.
  If a green fix turns out bigger than it looked while fixing, revert it and recolor it red.

## Report

Copy `report_template.html` next to this file; keep its CSS and script, replace only the data.
Same format as the `audit-gameplay` template, with two extra fields per item:

`[color, severity, title, where, problem, fix, state]` - `color` is `"g"` or `"r"`, `state` is `"fixed"` or `"todo"`.
For `fixed` items, the fix field says what was changed.

Also set the title, the header line (date, scope) and a new `DONE_KEY` per audit. Order the items in each system
red first (most severe first), then green. `TOP` = the worst red findings.

Write it in the user's language. Save it to `~/Downloads/audit-<topic>.html` and open it (`firefox <path> &`).
In the chat: counts (green fixed / red to decide) and the red findings one line each.
