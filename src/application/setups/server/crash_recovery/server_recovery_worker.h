#pragma once
#if CRASH_RECOVERY
#include <cstddef>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <optional>
#include <condition_variable>

#include "augs/filesystem/path_declaration.h"
#include "augs/network/port_type.h"

augs::path_type get_server_recovery_file_path(const port_type port);

/*
	Atomically writes raw bytes: serializes to a temp file then renames over the
	target, so a crash mid-write never leaves a truncated recovery file.
*/
void write_bytes_to_recovery_file(const std::vector<std::byte>& bytes, const augs::path_type& target_path);

/* Reads the whole recovery file. Returns std::nullopt if it is missing or unreadable. */
std::optional<std::vector<std::byte>> read_recovery_file(const augs::path_type& source_path);

/*
	One shared worker thread for all ranked server instances within the process.
	The thread is created lazily on the first push, so a process with recovery
	disabled everywhere never spawns it.

	Tasks are processed FIFO. Both writes and deletes are pushed here so a
	pending write can never finish AFTER a same-path delete - the main thread
	using std::filesystem directly would race the worker's atomic rename.
	Multiple writes for the same instance simply rewrite the same target path
	in order, so the on-disk file ends up reflecting the latest push - we never
	drop a snapshot because the previous write was still in flight (losing a
	round in the recovery file would defeat the point).
*/

class server_recovery_worker {
public:
	server_recovery_worker() = default;
	~server_recovery_worker();

	server_recovery_worker(const server_recovery_worker&) = delete;
	server_recovery_worker& operator=(const server_recovery_worker&) = delete;

	void push_write(
		std::vector<std::byte>&& bytes,
		augs::path_type target_path
	);

	void push_delete(augs::path_type target_path);

private:
	enum class task_kind {
		WRITE,
		DELETE_FILE
	};

	struct task {
		task_kind kind = task_kind::WRITE;
		std::vector<std::byte> bytes;
		augs::path_type target_path;
	};

	void enqueue(task&& t);
	void worker_loop();

	std::mutex lk;
	std::condition_variable cv;
	std::deque<task> queue;

	std::thread worker;
	bool started = false;
	bool stop = false;
};
#endif
