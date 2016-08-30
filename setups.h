#pragma once
#include <mutex>

class game_window;

class local_setup {
public:
	void process(game_window&);
};

class server_setup {
	std::mutex mtx;
	std::condition_variable cv;

	volatile bool server_ready = false;
public:
	void wait_for_listen_server();

	void process(game_window&);
};

class client_setup {
public:
	void process(game_window&);
};

class determinism_test_setup {
public:
	void process(game_window&);
};