#pragma once
#include <vector>

/* Undo/Redo interface applicable to interfaces like map editor or textbox */

namespace augs {
	namespace misc {

		/* "command" may itself implement memento or command pattern */

		template <class command>
		class undoredo {
			int level;
		public:
			std::vector<command> actions;
			undoredo() : level(0) {}

			bool undo() {
				if(next_to_undo() >= 0) {
					actions[next_to_undo()].execute(true);
					++level;
					return true;
				}
				return false;
			}

			bool redo() {
				if(next_to_redo() < int(actions.size())) {
					actions[next_to_redo()].execute(false);
					--level;
					return true;
				}
				return false;
			} 
			
			/* perform action and push it on stack */

			void action(const command& c) {
				if(state_level() > 0)
					actions.erase(actions.end() - state_level(), actions.end());
				
				/* For example, typing a letter is an action. 
				   You may want to have all the typed letters (until backspace) undoed by once,
				   so you have to implement bool command::include(command&) which decides whether the two commands can be merged into one bigger.
				   You want to merge "type 'a'" and "type 'b'" commands, 
				   and then command::include makes it "type 'ab'" command, but you don't want to merge "type 'a'" and "backspace" commands. 
				   */

				if(actions.empty() || !front().include(c)) /* if it didn't succeed to merge, create new command */
					actions.push_back(c);

				level = 0;
			}

			command& front() {
				return *actions.rbegin();
			}

			unsigned state_level() const {
				return level;
			}

			int next_to_undo() const {
				return actions.size() - state_level() - 1;
			}

			int next_to_redo() const {
				return next_to_undo() + 1;
			}

			void clear() {
				actions.clear();
				level = 0;
			}
		};
	}
}