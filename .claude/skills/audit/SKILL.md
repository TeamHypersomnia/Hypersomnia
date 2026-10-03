---
name: audit
description: Audit/review a Hypersomnia feature or system (recently added gameplay, rendering, UI code) against the project's review checklist - crash safety, simulation determinism, lag-robust RNG, comments, performance, web compatibility, parameter plumbing, logic vs audiovisual placement, deduplication, header hygiene, dead code, settings/editor UI. Use whenever the user says "zreviewuj mi X", "zaudytuj mi X", "zrób review X", "audyt X", "review X", "audit X".
---

# Feature audit

## Scoping

1. Find the code of the feature: `git log --oneline --grep=<keyword>` and `git show --stat <commits>`,
   then grep for the feature's structs/settings. Read the whole diff of the relevant commits, not only the final files.
2. If the user lists several independent systems, launch one subagent per system (in one message, in parallel).
   Give each subagent this skill's checklist path (`.claude/skills/audit/SKILL.md`) and the system's keywords/commits.
3. Report only. Do not edit code unless the user asks to fix.
   When the user says "fix everything", that means every finding, nits included.

## Checklist

1. **Crash safety** - null handles/pointers, out-of-range indices, division by zero, NaN/inf propagation,
   empty containers, dead entities referenced by id, asserts reachable from network/map data.
2. **Simulation determinism** - netcode relies on cross-platform simulation determinism (see README.md,
   "Networking is based on **cross-platform simulation determinism**"). Look for: non-`repro::` math
   (`std::sin`, `std::sqrt` etc.) in logic, iteration over unordered containers affecting logic,
   float state derived from audiovisual/inferred/client-only data, uninitialized fields, pointer-order dependence,
   dependence on frame rate or real time instead of the logic step.
3. **Lag-robust RNG quality** - even 100% deterministic RNG can hurt lag compensation. A seed must come from
   something known as soon as the relevant entity exists (e.g. the weapon's id plus a per-weapon shot counter),
   not from the world step number, because the step in which an action happens is unpredictable under network jitter.
   Example: the shell RNG seed should be derived from the weapon (id + shot index), so all random values of every
   subsequent shell are known once the weapon exists; if a shot lands in a different step due to jitter,
   the shell trajectory stays the same. Check every place where RNG is applied during gameplay.
4. **Comments** - not too many; they describe the current state and why it is so,
   never the iteration history ("we tried X, so now Y" - only say why Y).
5. **Performance** - CPU cost, GPU memory (FBOs, textures), extra passes over all entities of a type
   where a shorter path is evident, per-frame allocations. In extreme cases state could live in inferred
   instead of significant - only with good reasons.
6. **Web compatibility** - WebGL2/GLES3 limits (shader features, texture formats, render targets, extensions),
   Emscripten constraints (threads, file I/O, blocking calls), `#if PLATFORM_WEB` paths.
7. **Parameter plumbing** - special cases like "this one grenade type behaves differently in physics"
   should be a proper parameter on the definition/component, not a hardcoded type check buried in a system.
8. **Logic vs audiovisual placement** - new gameplay state lives in logic/components;
   what is purely visual/audio lives in audiovisual systems. Nothing audiovisual affects the simulation.
9. **Code quality: deduplication first** - aggressively extract repeated code into shared headers/functions.
10. **Header hygiene** - tweakable-parameter headers must not be included in widely shared headers;
    they belong mainly in the .cpp files that use them, to avoid recompiling everything.
    Structural headers are fine.
11. **Dead code** - evident leftovers of abandoned experiments. Not features merely turned off in settings.
12. **Logic state size and statelessness** - check that the solvable (significant) state does not bloat:
    every new field there costs snapshot size, cosmos copies and resimulation. Could the fields be arranged better
    (packed, merged, moved to a per-entity component only where needed)? Prefer statelessness: deduce as much as possible
    from the existing logic state instead of adding new fields that must be kept updated.
    Invariants (per-flavour definitions) are the opposite: writing to them is cheap, so boldly add fields there
    (e.g. a per-magazine threshold precomputed into `invariants::item` instead of scanning all gun flavours per frame).
13. **Predictability** - check how each new sound/effect/message is predicted: `always_predictable_v`,
    `never_predictable_v`, `predictable_only_by(subject)`. Something caused by a remote player (e.g. a kick,
    a shot) is often mispredicted and must be `predictable_only_by` its causer, otherwise spurious or doubled
    effects play on correction. Something fully decided by already-known state (e.g. shell floor hits decided at ejection)
    can be always predictable. Also check that client-predicted and referential clocks/values are not mixed
    (e.g. comparing a referential timestamp with the predicted cosmos time).
14. **Settings and editor UI/UX** - new settings and related editor controls are thematically grouped,
    not cluttered, and use imgui properly: grouping, indentation, collapsing headers/tree nodes,
    showing only what is relevant/enabled.

## Report format

Per checklist point: findings with `file:line`, severity, a concrete problem and a concrete fix.
Skip points with nothing to report, but list them in one line as "clean". Rank the most severe findings first.

**The final report is an HTML page, in exactly the format of `report_template.html` next to this file** -
a real report from 2026-09-30, kept as the example. Copy it, keep its CSS and script, and replace only the data:

- `SYSTEMS` - one entry per audited system, one tab each: `{ id, name, clean, items }`.
  `clean` is the one line listing the checklist points with nothing to report.
  Every item is `[severity, title, where, problem, fix]` - `where` lists `file:line` in `<code>`,
  severity is one of `crash`, `desync`, `lag`, `bug`, `perf`, `quality`, `nit`, `decision`
  (`decision` for what the user has to settle, e.g. intended behavior vs a bug).
- `TOP` - `[system id, item index]` pairs of the worst findings across all systems, most severe first, shown in the first tab.
- The page title, the header line (date, checklist path) and the `DONE_KEY` of localStorage - a new key per audit.

Write it in the user's language. Save it to `~/Downloads/audit-<topic>.html` and open it in Firefox
(`firefox <path> &`). The page has severity filters and per-finding "done" checkboxes kept in the browser.
Then summarize the worst findings in the chat as well.
