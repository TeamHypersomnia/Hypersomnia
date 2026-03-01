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
  - **Metropolis**: Rebalances bots' `patrol_letter` distribution — if any bot's letter is over-covered by 2+ compared to the least-covered letter (via `find_least_assigned_bombsite`), it switches. Skipped when bomb is planted.
- **Per-bot** initialization is in `update_arena_mode_ai()` PHASE 0:
  - If `patrol_letter` is `COUNT`, Metropolis bots use `find_least_assigned_bombsite()`, others pick randomly.
- Uses `marker_letter_type::COUNT` as sentinel for "not yet assigned".

### 4. Faction Matching Fix (`faction_type.h`)
- `is_waypoint_for_faction()` now treats both `faction_type::ANY` and `faction_type::DEFAULT` as applicable for both factions, per the specification.

### 5. Plant Pathfinding Optimization (`calc_pathfinding_request.hpp`)
- Plant behavior now uses the gathered `bombsite_mappings` to look up bombsite entities by letter directly, instead of iterating all `area_marker` entities.
- Removed the per-entity faction check (bombsite mappings are per-team anyway).

### 6. Bombsite Rebalancing Helpers (`ai_waypoint_helpers.hpp`)
- `find_least_assigned_bombsite()` now uses gathered bombsite mappings to only consider available letters.
- Added `choose_random_bombsite()` for random bombsite selection from available letters.

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
- `update_arena_mode_ai()` runs per-bot. PHASE 0 handles per-bot `patrol_letter` initialization using `find_least_assigned_bombsite()` for Metropolis or random pick for others.
- `find_least_assigned_bombsite()` is always used for Metropolis — both at initialization and for continuous rebalancing. When a bot dies, its waypoint assignments clear, the letter's count drops, and `update_arena_mode_ai_team` detects the imbalance (difference ≥ 2) and rebalances. Skipped when bomb is planted.
- The random selection for Resistance ensures unpredictability — the attacking team doesn't always go to the same site.
