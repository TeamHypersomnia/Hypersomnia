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
	Called by the runner as soon as a single match resolves. The coordinator's
	implementation serializes concurrent invocations and persists the state
	atomically before returning, so a crash after this call sees the result on
	resume - no replay of finished matches.
*/
using tournament_commit_match_result_fn = std::function<
	void(tournament_match&, uint32_t winner_team_index, float duration_secs)
>;

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
	)> run_stage;

	/*
		Called once per port whose per-port crash-recovery file should be removed.
		Used at two points:
		  - Just after a fresh pair_teams() to wipe stale snapshots that any prior
		    process left at the ports the coordinator is about to bind. Without this,
		    a port the new tournament didn't use in stage 0 but does use in stage 1
		    (e.g. after byes inflate the field) would inherit an unrelated old match's
		    snapshot and resume the wrong arena.
		  - For matches a resumed state.json reports as already resolved, so their
		    stale snapshot doesn't fight the next stage's bind on the same port.
		On resume into a stage with unresolved matches, the wipe is intentionally
		NOT called - those per-port files ARE the mid-match resume payload.
	*/
	std::function<void(uint16_t port)> wipe_recovery_for_port;

	/*
		Optional - blocks until the per-port crash-recovery worker has drained its
		queue. Called between stages so any in-flight delete from a just-finished
		match is on disk before the next stage's server_setup constructor calls
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
};

class tournament_coordinator {
public:
	tournament_coordinator(
		tournament_config config,
		augs::path_type state_file_path,
		tournament_coordinator_dependencies deps
	);

	void run();

private:
	bool finished() const;
	void initialize_state_if_fresh();
	void pair_teams();
	void run_current_stage();
	void update_skills();
	void eliminate_losers();
	void advance_stage();
	void persist() const;
	void declare_winner() const;
	void cleanup() const;

	std::string make_server_name_for(const tournament_match&) const;

	void commit_match_result(tournament_match& m, uint32_t winner_team_index, float duration_secs);

	tournament_config cfg;
	tournament_state state;
	augs::path_type state_path;
	tournament_coordinator_dependencies deps;

	mutable std::mutex state_lk;
};

#endif
