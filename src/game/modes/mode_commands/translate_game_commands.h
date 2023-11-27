#pragma once
#include "augs/network/network_types.h"

/*
	round_restart
	round_restart_nofreeze
	rich

	etc., just lowercase versions
*/

enum class no_arg_game_command {
	ROUND_RESTART,
	ROUND_RESTART_NOFREEZE,
	RICH
};

/*
	pos nickname x y [angle]
	examples:

	pos "Pythagoras" 20 10
	pos Pythagoras 20 10 90
*/

struct pos_game_command {
	client_nickname_type nickname;
	transformr new_transform;
};

struct plant_game_command {
	transformr bomb_transform;
};

/*
	eq nickname items...
	examples:

	eq "Pythagoras" backpack armor
	eq Pythagoras armor baka47 szturm backpack force_grenade
	eq Pythagoras

	(can be empty too)
*/

struct eq_game_command {
	client_nickname_type nickname;
	augs::constant_size_vector<augs::constant_size_string<30>, 20> items;
};

/*
	all_commands is a line-breaked sequence of commands, e.g.

	round_restart
	pos Pythagoras 10 10 45
	rich
	pos slow3586 505 5
	pos Pythagoras 0 0

	Nickname will or will not be delimited by ", depending if it has a space. If there's no ",
	assume all characters until the next space consitute the nickname.

	callback is a generic lambda and will be called on eq_game_command,
	pos_game_command or no_arg_game_command respectively depending on the type of command.
*/

template <class S, class F>
void translate_game_commands(
    const S& all_commands,
    F&& callback
) {
    std::istringstream stream(all_commands);
    std::string line;

    // Lambda to read a nickname
    auto read_nickname = [](std::istringstream& stream) -> client_nickname_type {
		std::string nickname;

        stream >> std::ws;
        
        if (stream.peek() == '"') {
            stream.get(); // Skip the opening quote
            std::getline(stream, nickname, '"');
            // Skip the space after closing quote
            stream >> std::ws;
        }
        else {
            stream >> nickname;
        }
        return nickname;
    };

    while (std::getline(stream, line)) {
        std::istringstream line_stream(line);
        std::string command;
        line_stream >> command;

        if (command == "round_restart") {
            callback(no_arg_game_command::ROUND_RESTART);
        }
		else if (command == "round_restart_nofreeze") {
			callback(no_arg_game_command::ROUND_RESTART_NOFREEZE);
		}
        else if (command == "rich") {
            callback(no_arg_game_command::RICH);
        }
        else if (command == "plant") {
            plant_game_command cmd;

			line_stream >> cmd.bomb_transform.pos.x >> cmd.bomb_transform.pos.y;
            if (!(line_stream >> cmd.bomb_transform.rotation)) {
                cmd.bomb_transform.rotation = 0; // Default angle if not provided
            }

            callback(cmd);
        }
        else if (command == "pos") {
            pos_game_command cmd;
            cmd.nickname = read_nickname(line_stream);

            // Read transform
			line_stream >> cmd.new_transform.pos.x >> cmd.new_transform.pos.y;
            if (!(line_stream >> cmd.new_transform.rotation)) {
                cmd.new_transform.rotation = 0; // Default angle if not provided
            }

            callback(cmd);
        }
        else if (command == "eq") {
            eq_game_command cmd;
            cmd.nickname = read_nickname(line_stream);

            // Read items
            std::string item;
            while (line_stream >> item) {
				if (cmd.items.size() >= cmd.items.max_size()) {
					break;
				}

                cmd.items.push_back(item);
            }

            callback(cmd);
        }
    }
}
