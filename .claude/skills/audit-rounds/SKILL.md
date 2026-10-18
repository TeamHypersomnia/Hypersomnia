---
name: audit-rounds
description: Running audits inside a longer working session and acting on their findings without going in circles - keeps one decisions document, checks every fix against what was already cut or decided, and shows the state of each finding as a status page (checkmarks, green/red, loop markers). Use whenever, after other work in the session, the user asks to run or launch audits ("puść audyt", "audyt diffa", "puść te audyty", "zaudytuj"), asks what an audit found, asks to fix after an audit ("popraw po audycie"), or asks whether we are going in circles ("czy kręcimy się w kółko"). Not for a session whose only prompt is the audit itself - that one just audits (see the `audit-gameplay` skill or the given prompt).
---

# Audit rounds in a working session

**The most important thing in these rounds is cutting complexity.** Every finding is first a chance to remove code
or behavior; a fix that adds a mechanism is the last resort. The status page proposes a cut for each item where one
exists, and complexity is what decides the row's color.

Audits in the middle of a long session go wrong in one way above all: a finding is the effect of something cut earlier,
and the fix quietly brings back a smaller copy of the cut mechanism (known_servers → learn_server_key → pin_proven_key;
a held cert → hold_account). Or a fix makes new bugs that the next audit finds (a result stored on GET → DoS). This
skill keeps that from happening.

## Before launching audits

1. Open or create the session's decisions document - one markdown file the user can read, e.g.
   `~/Downloads/audyty/<name>/USTALENIA.md`, with: rules against circling, what is decided, what is open, and the
   history of every loop. Copy every audit report into a `raporty/` folder next to it, so none gets lost in a compact.
   If an old report would be overwritten, move it aside with its date in the name (never delete).
2. Audit prompts from the user's files go verbatim. If a prompt names something that no longer exists
   (e.g. a removed mechanism), do not edit the prompt - append a short "current state" context block to it, listing
   what was removed and which decisions are intentional (no backward compatibility, etc.), so the auditor does not
   flag decided behavior or hunt for deleted code. Say you did this.
3. Every audit runs as the `auditor` agent (`.claude/agents/auditor.md` - read-only, writes one Markdown report in
   its own finding format; you read it and summarize it for the user, never an HTML report here).
4. Diff audits: one `auditor` over the exact ranges (game commit range, the site's commits since the same time, the
   netcode fork's commits).
5. Long waits: wrap every test command in a short `timeout`, check background shells about every minute (builds
   are the only exception), never pipe a script that backgrounds a server into `tail`.

## A loop is a decision to make, not a verdict

Coming back to an earlier state is often the right answer. A loop means the choice between the two states was never
made for good - so make it once: lay out both sides in a sentence each, recommend one, and record the decision and
the reason in the decisions document, so neither side comes back again. Never argue against an option only because
"it is a return" or "it is a loop".

## Before fixing anything an audit found

For every finding ask, in this order, and write the answer into the decisions document:

1. **Is it the effect of something we cut or decided?** If yes, do not quietly bring back a smaller copy of the
   mechanism - decide openly between the two states (see above). Prefer removing the behavior that needed it (the auto-relaunch with a saved password was removed, not
   pinned), or moving it to where the problem does not exist (the password prompt after the waits, the list cache
   only in the GUI, instance labels instead of per-instance keys).
2. **Does the fix add or remove complexity?** A fix that adds a map, a flag, a second code path or a new endpoint
   needs a reason the user would agree with. Say what it adds.
3. **Is it the user's call?** Product and UX (what a player or an operator sees or must do) go to the user, briefly,
   with a recommendation. Technical details with no visible effect are decided without asking - and written down.
4. **Can the fix itself break something?** A response stored on GET, a key per match that is not registered anywhere,
   a dedupe broken by re-signing: check the new code path against abuse and against the deployment (what the operator
   must now register or configure).

Then fix, test right away (on screen or with the scenario scripts), commit, and update the document.

## The status page

When the user asks where things stand after an audit ("pokaż mi ustalenia audytu", "w formie html", "czy kręcimy się
w kółko"), publish one HTML page (the Artifact tool; `status-page-example.html` next to this file is a finished
example) with:

- **Summary counts:** green, red, and "you addressed N of M".
- **A table, one row per finding:** `#` · the finding in one line · **Ty** (✓ odniosłeś się / — jeszcze nie: whether
  the user has decided or commented on it) · a label (zostaje, wycięte, zrobić inaczej, pominięte, a replacement such
  as "etykiety", and ↻ when the item comes back to something already cut or changed) · the state and assessment after
  all decisions, in a sentence or two.
- **Row color - decided by complexity first,** by the green/red rules in `.claude/skills/audit-green-red/SKILL.md`
  ("Green or red"). On top of them, here: a row is also green when the fix is planned to be done that way (even if not
  in the code yet), and red when it goes in circles or is still broken / unresolved (skipped counts as unresolved).
- **A cut proposal on every row where one exists** ("wyciąć X", "zamiast Y przenieść do Z") - also on green rows,
  when something there could still go.
- **A "Gdzie się kręciliśmy" section:** one card per loop, the chain of states with arrows
  (A → B → C), and a short conclusion on what kind of mistake caused the loops.
- Light and dark themes, phone width, tokens on `:root` (see the example).

Republish the same file path on later updates, so the link stays the same.

## When the user talks about audits

Whenever the user mentions audits in a longer session (planning them, their order, their results), point to this
skill's two rules in one line: every fix is checked against what was already cut, and the status page shows loops in
red. Warn right away when a planned fix brings back something from the loop history in the decisions document.
