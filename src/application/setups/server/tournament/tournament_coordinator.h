#pragma once
#if HEADLESS
#include <cstdint>
#include <functional>
#include <mutex>
#include <vector>
#include "augs/filesystem/path_declaration.h"
#include "application/setups/server/tournament/tournament_config.h"
#include "application/setups/server/tournament/tournament_state.h"

/*
	On top of a round's time to enter: a player coming at the very minute still gets in.
*/

inline constexpr uint32_t tournament_round_grace_secs_v = 2 * 60;

/*
	Called by the runner as soon as a single match resolves. The coordinator's
	implementation serializes concurrent invocations and persists the state
	atomically before returning, so a crash after this call sees the result on
	resume - no replay of finished matches.

	An open match of the first round has its team_a/b_player_ids filled by the runner first - the coordinator
	makes its teams of them. Closed by Close the round (or its time to enter): a walkover of team a (whoever waited), or with both
	rosters empty, an empty server that's gone.
*/

struct tournament_match_outcome {
	bool team_a_won = true;
	float duration_secs = 0.f;
	uint32_t team_a_score = 0;
	uint32_t team_b_score = 0;
	bool walkover = false;

	/*
		Join timeout with nobody on the server: no winner - both teams out.
	*/

	bool nobody_came = false;

	/*
		An open match's teams - set into the match under the coordinator's lock, as other matches' threads
		may be saving the state meanwhile.
	*/

	std::vector<account_id_string> open_team_a;
	std::vector<account_id_string> open_team_b;

	/*
		Their nicknames, in the same order - the later rounds' servers are named by them, not by the account ids.
	*/

	std::vector<std::string> open_names_a;
	std::vector<std::string> open_names_b;
};

using tournament_commit_match_result_fn = std::function<void(tournament_match&, const tournament_match_outcome&)>;

struct tournament_coordinator_dependencies {
	/*
		Runs the listed matches concurrently and blocks until all complete.
		Each per-match server should call commit_match_result as soon as its
		match decides; the coordinator handles state-locking and persistence.

		`should_interrupt` lets the coordinator bail out cleanly on SIGINT.
	*/
	std::function<void(
		std::vector<tournament_match>&,
		tournament_commit_match_result_fn
	)> run_bracket_round;

	/*
		Called once per port whose per-port crash-recovery file should be removed.
		Used at two points:
		  - Just after a fresh pair_teams() to wipe stale snapshots that any prior
		    process left at the ports the coordinator is about to bind. Without this,
		    a port the new tournament didn't use in bracket round 0 but does use in bracket round 1
		    (e.g. after byes inflate the field) would inherit an unrelated old match's
		    snapshot and resume the wrong arena.
		  - For matches a resumed state.json reports as already resolved, so their
		    stale snapshot doesn't fight the next bracket round's bind on the same port.
		On resume into a bracket round with unresolved matches, the wipe is intentionally
		NOT called - those per-port files ARE the mid-match resume payload.
	*/
	std::function<void(uint16_t port)> wipe_recovery_for_port;

	/*
		Optional - blocks until the per-port crash-recovery worker has drained its
		queue. Called between bracket rounds so any in-flight delete from a just-finished
		match is on disk before the next bracket round's server_setup constructor calls
		load() on the same path. Without this, the new instance can read a stale
		snapshot the worker hadn't gotten around to deleting and resume the wrong
		match.
	*/
	std::function<void()> wait_for_pending_recovery_io;

	/*
		The base port the first instance binds; subsequent instances increment from here.
	*/
	uint16_t base_port = 0;

	/*
		Returns true if the user has signalled a shutdown (SIGINT etc.).
	*/
	std::function<bool()> should_interrupt;

	/*
		Optional - a copy of the state every time it's saved, for the live page (served from another thread).
	*/
	std::function<void(const tournament_state&)> on_state_changed;
};

class tournament_coordinator {
public:
	tournament_coordinator(
		tournament_config config,
		augs::path_type state_file_path,
		tournament_coordinator_dependencies deps
	);

	/*
		False if the same tournament (the same flags) was already played to the end here - it doesn't start again.
	*/

	bool run();

	/*
		After run() returned false: the results of this tournament, played before.
	*/

	augs::path_type completed_archive;

	/*
		After run() returned true: stopped because a match ended without a result - not over.
	*/

	bool aborted = false;

	/*
		Whether a player of the tournament (is_player tells by its id as written in the state) has no match of its own
		to play now - out, waiting on a bye, or its match of this round decided - so may watch the others.
		Called from the match servers' threads.
	*/

	bool may_watch_others(const std::function<bool(const std::string&)>& is_player) const;

	/*
		A listed nickname taken by the first account to come with it (placeholder: nick_<nickname>) - from now on that
		player is that account, in this match, its team and the history. Called from the match servers' threads.
	*/

	bool claim_listed_nickname(tournament_match&, const account_id_string& placeholder, const account_id_string& account);

	/*
		When the current round's matches stop waiting for their players (seconds since the epoch, 0: never) -
		round 1 from the tournament's start, the next ones from the previous round's end. A restart (a crash) gives
		the round its whole time again, from the restart.
	*/

	int64_t current_bracket_round_deadline() const;

private:
	bool finished() const;
	void initialize_state_if_fresh();
	void pair_teams();
	void run_current_bracket_round();
	void update_skills();
	void eliminate_losers();
	void advance_bracket_round();
	void persist() const;
	void declare_winner() const;
	void cleanup() const;

	std::string make_server_name_for(const tournament_match&) const;
	void pair_open_first_bracket_round();
	std::optional<augs::path_type> find_completed_archive() const;

	void commit_match_result(tournament_match& m, const tournament_match_outcome&);
	uint32_t add_team_of(const std::vector<account_id_string>&, const std::vector<std::string>& nicknames);

	tournament_config cfg;
	tournament_state state;

	/*
		When the current round started in this run - not saved: a restart counts the round's time from the restart.
	*/

	uint64_t bracket_round_started_at = 0;
	augs::path_type state_path;
	bool resumed = false;
	tournament_coordinator_dependencies deps;

	mutable std::mutex state_lk;
};

#endif
