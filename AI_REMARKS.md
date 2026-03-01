# AI Remarks - Bombsite Selection & Rebalancing

## Summary of Previous Bombsite Selection Behavior

### Resistance (Attackers)
- `chosen_bombsite` in `arena_mode_ai_team_state` was hardcoded to `marker_letter_type::A` on round reset.
- No random selection occurred at round start — Resistance bots always targeted bombsite A initially.
- When the bomb was actually planted, the code detected which bombsite it was planted at and updated `chosen_bombsite` to match the planted site.
- The plant pathfinding (`calc_pathfinding_request.hpp`) iterated **all** `area_marker_type::BOMBSITE` entities each time it needed to find the target, filtering by letter, faction, and checking AABB occupancy.

### Metropolis (Defenders)
- Each bot had `patrol_letter` in `arena_mode_ai_state`, defaulting to `marker_letter_type::A` on round reset.
- No distribution across bombsite letters occurred at round start — all Metropolis bots defaulted to patrolling letter A waypoints.
- When the bomb was planted, ALL Metropolis bots got their `patrol_letter` set to the planted bombsite letter (so they all rush the same site).
- `find_least_assigned_bombsite()` existed in `ai_waypoint_helpers.hpp` but was **never called** anywhere.

### Waypoint Faction Filtering
- `is_waypoint_for_faction()` only treated `faction_type::ANY` as "applicable for both factions".
- `faction_type::DEFAULT` was NOT treated as universal, despite the spec requiring it.

## Changes Made

### 1. Bombsite Mapping Data Structure (`arena_mode_ai_structs.h`)
- Added `bombsite_mapping` struct: `{ marker_letter_type letter; std::vector<entity_id> bombsite_ids; }`
- Added `std::vector<bombsite_mapping> bombsite_mappings` to `arena_mode_ai_team_state`
- Added helper methods: `find_bombsite_ids(letter)`, `has_bombsite_letter(letter)`, `get_available_bombsite_letters()`

### 2. Bombsite Gathering at Round Start (`ai_waypoint_helpers.hpp`)
- `gather_waypoints_for_team()` now also iterates `area_marker_type::BOMBSITE` entities and builds the letter → entity ID mappings.
- This is called at round start for each faction.

### 3. Stateless Bombsite Selection (`arena_mode_ai.cpp`)
- **Team-level** logic is in `update_arena_mode_ai_team()`, called once per step per faction before the per-bot loop:
  - **Resistance**: Picks `chosen_bombsite` randomly if it's still `COUNT` (unset).
  - **Metropolis**: Rebalances bots' `patrol_letter` distribution — uses `find_most_assigned_bombsite` and `find_least_assigned_bombsite` (both wrappers around a generic `find_bombsite_by_assignment` with template comparator). If the most-assigned letter has 2+ more than the least-assigned, switches one bot (using the `example_bot` from the result). Skipped when bomb is planted.
- **Per-bot** initialization is in `update_arena_mode_ai()` PHASE 0:
  - Only Metropolis bots get `patrol_letter` initialized (via `find_least_assigned_bombsite()`). `patrol_letter` is only used for Metropolis bombsite defense distribution.
- Uses `marker_letter_type::COUNT` as sentinel for "not yet assigned".

### 4. Faction Matching Fix (`faction_type.h`)
- `is_waypoint_for_faction()` now treats both `faction_type::ANY` and `faction_type::DEFAULT` as applicable for both factions, per the specification.

### 5. Plant Pathfinding Optimization (`calc_pathfinding_request.hpp`)
- Plant behavior now uses the gathered `bombsite_mappings` to look up bombsite entities by letter directly, instead of iterating all `area_marker` entities.
- Removed the per-entity faction check (bombsite mappings are per-team anyway).

### 6. Bombsite Rebalancing Helpers (`ai_waypoint_helpers.hpp`)
- `bombsite_assignment_info` struct holds letter, count, and example_bot.
- `find_bombsite_by_assignment()` is a generic template function with a comparator parameter that finds the best bombsite letter by assignment count.
- `find_least_assigned_bombsite()` and `find_most_assigned_bombsite()` are wrappers using `std::less` and `std::greater`.
- Added `choose_random_bombsite()` for random bombsite selection from available letters.

### 7. Dead Bot Waypoint Assignment Fix (`arena_mode.cpp`)
- Waypoint reassignment loops now skip dead/unconscious bots via `sentient_and_conscious()`.
- When a bot dies, its waypoint assignments are no longer counted, allowing `find_least_assigned_bombsite` counts to decrease properly.

## What Was Already Done (from the spec)
- Waypoint types (`BOT_WAYPOINT_PATROL`, `BOT_WAYPOINT_PUSH`) with `camp` flag and faction — already implemented.
- Patrol behavior with camp/twitch mechanics — already implemented.
- Push waypoint logic — already implemented.
- Defuse mission assignment — already implemented.
- Bomb retrieval mission — already implemented.
- Combat behavior tree — already implemented.
- Post-bomb-plant bombsite detection and AI redirection — already implemented.

## Architecture Notes
- The `bombsite_mappings` live in `arena_mode_ai_arena_meta`, a team-agnostic struct stored once in `arena_mode`. Since bombsites are global map features (not faction-specific), they are gathered once at round start via `gather_bombsite_mappings()` and passed to AI functions as needed.
- `update_arena_mode_ai_team()` runs once per step per faction (before the per-bot loop). It handles team-level decisions: Resistance `chosen_bombsite` initialization and Metropolis patrol rebalancing.
- `update_arena_mode_ai()` runs per-bot. PHASE 0 handles per-bot `patrol_letter` initialization using `find_least_assigned_bombsite()` for Metropolis only. `patrol_letter` is only used in the context of Metropolis (bombsite defense distribution).
- `find_least_assigned_bombsite()` and `find_most_assigned_bombsite()` are both wrappers around `find_bombsite_by_assignment()` with template comparator. They return `bombsite_assignment_info` with letter, count, and example_bot. Used for Metropolis initialization and continuous rebalancing. Skipped when bomb is planted.
- Waypoint reassignment loops in `arena_mode.cpp` now skip dead/unconscious bots via `sentient_and_conscious()`, so when a bot dies its waypoints are freed and assignment counts decrease properly.
