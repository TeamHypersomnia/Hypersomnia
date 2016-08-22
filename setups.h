#pragma once

class game_window;

class local_setup {
public:
	void process(game_window&);
};

class server_setup {
public:
	void process(game_window&);
};

class client_setup {
public:
	void process(game_window&);
};